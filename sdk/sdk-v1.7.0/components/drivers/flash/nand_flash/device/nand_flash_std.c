/* Generic NAND flash driver, only support standard spi mode */
#include "flash_dev.h"
#include "udevice.h"
#include "spi.h"
#include "sysctl.h"
#include "flash.h"
#include "aiva_sleep.h"
#include "aiva_malloc.h"
#include "syslog.h"
#include "nand_comm.h"
#include <string.h>


/* clang-format off */

#define GET_PAGE_ADDR(blk, page)     ((((blk) << 6) | ((page) & 0x3f)) & 0xffff)
#define GET_COL_ADDR(page_offset)    ((page_offset) & 0xfff)

/* Command codes for the flash_command routine */
#define FLASH_RESET             0xFF    /* reset flash */
#define FLASH_RDID              0x9F    /* read manufacturer and product id */
#define FLASH_GFEAT             0x0F    /* read status/get feature option */
#define FLASH_SFEAT             0x1F    /* write status/set feature option */
#   define FEATURE_PROT_ADDR    0xA0    /* protection register */
#   define FEATURE_FEAT_ADDR    0xB0    /* configuration register */
#   define FEATURE_STAT_ADDR    0xC0    /* status register */
#define FLASH_WREN              0x06    /* set write enable latch */
#define FLASH_WRDI              0x04    /* reset write enable latch */
#define FLASH_BERASE            0xD8    /* erase one block in memory array */

#define FLASH_PROG              0x02    /* program load data to cache */
#define FLASH_QPROG             0x32    /* quad program load data to cache */
#define FLASH_PEXEC             0x10    /* program cache data to memory array */
#define FLASH_PROG_RAN          0x84    /* program load data to cache at offset */
#define FLASH_QPROG_RAN         0x34    /* quad program load data to cache at offset */

#define FLASH_PREAD             0x13    /* read from memory array to cache */
#define FLASH_READ              0x03    /* read data from cache */
#define FLASH_READ_FAST         0x0B    /* read data from cache */
#define FLASH_QREAD_FAST        0xEB    /* quad-read data from cache */

#define FEATURE_STAT_ENH        0x30
#define FEATURE_STAT_AUX        0xF0

#define STAT_ECC_MASK           0x30
#define STAT_ECC_UNCORR         0x20    /* uncorrectable error */
#define STAT_PFAIL              0x8     /* program fail */
#define STAT_EFAIL              0x4     /* erase fail */
#define STAT_WEL                0x2     /* write enable latch */
#define STAT_OIP                0x1     /* operation in progress */

#define FEAT_ECC_EN             (1u << 4)
#define FEAT_BUF_EN             (1u << 3)
#define FEAT_QUAD_EN            (1u << 0)

#define FLASH_SELECT_DIE        0xC2
#define FLASH_BAD_BLOCK_MARKER  0x00

typedef enum read_mode {
    BUFFER_MODE = 0,
    CONTINUOUS_MODE = 1,
} read_mode_t;

typedef int (*nand_read_page_func_t)(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *rx_buf, int rx_len, int check_ecc, read_mode_t readmode);
typedef int (*nand_page_program_func_t)(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *tx_buf, int tx_len);

typedef struct nand_flash_priv_params {
    bblk_lut_t bblk_lut;
    uint32_t   block_size;
    uint32_t   page_size;
    uint32_t   extra_size;
    uint32_t   die_size;
    uint8_t    current_die_index;
    uint8_t    feature;
    uint8_t    quad_mode;
    uint8_t    quad_mode_should_write_regs;
} nand_flash_priv_params_t;


static int nand_std_wait_for_ready(flash_dev_t *dev);
static int nand_std_page_program(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *tx_buf, int tx_len);
static int nand_std_page_read(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *rx_buf, int rx_len, int check_ecc, read_mode_t readmode);
static int nand_std_select_die(flash_dev_t *dev, uint8_t die_num);

static int nand_std_receive_data(flash_dev_t *dev, uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
    int ret;
    spi_device_num_t spi_num = dev->cfg.spi_num;
    spi_chip_select_t spi_cs = dev->cfg.spi_cs;

    spi_init(spi_num, SPI_WORK_MODE_0, SPI_FF_STANDARD, FIFO_W8);
#ifdef FLASH_USE_DMA
    ret = spi_receive_data_standard_dma(spi_num, spi_cs, cmd_buff, cmd_len, rx_buff, rx_len);
#else
    ret = spi_receive_data_standard(spi_num, spi_cs, cmd_buff, cmd_len, rx_buff, rx_len);
#endif
    if (ret < 0) {
        return -FLASH_ERROR_SPI_TRANS;
    }
    return FLASH_OK;
}

static int nand_std_send_data(flash_dev_t *dev, uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    int ret;
    spi_device_num_t spi_num = dev->cfg.spi_num;
    spi_chip_select_t spi_cs = dev->cfg.spi_cs;

    spi_init(spi_num, SPI_WORK_MODE_0, SPI_FF_STANDARD, FIFO_W8);
#ifdef FLASH_USE_DMA
    ret = spi_send_data_standard_dma(spi_num, spi_cs, cmd_buff, cmd_len, tx_buff, tx_len);
#else
    ret = spi_send_data_standard(spi_num, spi_cs, cmd_buff, cmd_len, tx_buff, tx_len);
#endif 
    if (ret < 0) {
        return -FLASH_ERROR_SPI_TRANS;
    }
    return FLASH_OK;
}

static int nand_std_receive_data_enhanced(flash_dev_t *dev, uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len, read_mode_t readmode)
{
    int ret;
    spi_device_num_t spi_num = dev->cfg.spi_num;
    spi_chip_select_t spi_cs = dev->cfg.spi_cs;

    spi_init(spi_num, SPI_WORK_MODE_0, SPI_FF_QUAD, FIFO_W32);
    uint32_t address_len = (readmode == BUFFER_MODE) ? 16 : 0;
    uint32_t wait_cycles = (readmode == BUFFER_MODE) ? 4 : 12;
    spi_init_non_standard(spi_num, 8/*instrction length*/, address_len/*address length*/, wait_cycles/*wait cycles*/,
                          SPI_AITM_ADDR_STANDARD/*spi address trans mode*/);
#ifndef FLASH_USE_DMA
    ret = spi_receive_data_multiple(spi_num, spi_cs, cmd_buff, cmd_len, rx_buff, rx_len);
#else
    ret = spi_receive_data_multiple_dma(spi_num, spi_cs, cmd_buff, cmd_len, rx_buff, rx_len);
#endif
    if (ret < 0) {
        LOGE(__func__, "spi receive error %d.", ret);
        return -FLASH_ERROR_SPI_TRANS;
    }
    return FLASH_OK;
}

static int nand_std_send_data_enhanced(flash_dev_t *dev, uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    int ret;
    spi_device_num_t spi_num = dev->cfg.spi_num;
    spi_chip_select_t spi_cs = dev->cfg.spi_cs;

    spi_init(spi_num, SPI_WORK_MODE_0, SPI_FF_QUAD, FIFO_W32);
    spi_init_non_standard(spi_num, 8/*instrction length*/, 16/*address length*/, 0/*wait cycles*/,
                          SPI_AITM_STANDARD/*spi address trans mode*/);
#ifndef FLASH_USE_DMA
    ret = spi_send_data_multiple(spi_num, spi_cs, cmd_buff, cmd_len, tx_buff, tx_len);
#else
    ret = spi_send_data_multiple_dma(spi_num, spi_cs, cmd_buff, cmd_len, tx_buff, tx_len);
#endif
    if (ret < 0) {
        return -FLASH_ERROR_SPI_TRANS;
    }

    return FLASH_OK;
}

static int nand_std_read_status(flash_dev_t *dev, uint8_t reg, uint8_t *reg_data)
{
    int ret;
    uint8_t cmd[2] = {FLASH_GFEAT, reg};
    
    ret = nand_std_receive_data(dev, cmd, 2, reg_data, 1);
    CHECK_RET(ret);

    return ret;
}

static int nand_std_is_busy(flash_dev_t *dev)
{
    uint8_t status = 0;
    
    nand_std_read_status(dev, FEATURE_STAT_ADDR, &status);
    if (status & 0x1) {
        return 1;
    }
    return 0;
}

static int nand_std_wait_for_ready(flash_dev_t *dev)
{
    uint32_t timeout;

    timeout = 20 * 1000 * 100; // 20 sec

    while(timeout--) {
        if (!nand_std_is_busy(dev)) {
            return FLASH_OK;
        }
        /*aiva_usleep(10);*/
    }
    return -FLASH_ERROR_TIMEOUT;
}

static int nand_std_write_status(flash_dev_t *dev, uint8_t reg, uint8_t reg_data)
{
    int ret;
    uint8_t cmd[3] = {FLASH_SFEAT, reg, reg_data};

    ret = nand_std_send_data(dev, cmd, 3, 0, 0);
    CHECK_RET(ret);
    
    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    return ret;
}

static int nand_std_reset(flash_dev_t *dev)
{
    int ret;
    uint8_t cmd[1] = {FLASH_RESET};
    
    ret = nand_std_send_data(dev, cmd, 1, 0, 0);
    aiva_usleep(100);

    return ret;
}


// nand_flash_write_enable() must be called before any change to the 
// device such as write, erase. It also unlocks the blocks.
static int nand_std_write_enable(flash_dev_t *dev)
{
    int ret;
    uint8_t prot;

    /* make sure it is not locked firstly */
    ret = nand_std_read_status(dev, FEATURE_PROT_ADDR, &prot);
    CHECK_RET(ret);

    if (prot != 0) {
        prot = 0;
        ret = nand_std_write_status(dev, FEATURE_PROT_ADDR, prot);
        CHECK_RET(ret);
    }

    uint8_t cmd[1] = {FLASH_WREN};

    ret = nand_std_send_data(dev, cmd, 1, 0, 0);
    CHECK_RET(ret);

    ret = nand_std_wait_for_ready(dev);
    if (ret < 0) {
        return  ret;
    }
    
    ret = nand_std_read_status(dev, FEATURE_STAT_ADDR, &prot);
    CHECK_RET(ret);

    if (!(prot & STAT_WEL)) {
        return -FLASH_ERROR_STATUS;
    }

    return ret;
}

/* Set ECC-E in configuration register */
static int nand_std_enable_ecc(flash_dev_t *dev)
{
    int ret;
    uint8_t status;

    ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
    CHECK_RET(ret);

    status |= FEAT_ECC_EN;

    ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
    CHECK_RET(ret);

    return FLASH_OK;
}

/*Clear ECC-E in configuration register*/
static int nand_std_disable_ecc(flash_dev_t *dev)
{
    int ret;
    uint8_t status;

    ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
    CHECK_RET(ret);

    status &= (~FEAT_ECC_EN);

    ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
    CHECK_RET(ret);

    return FLASH_OK;
}

/* Set BUF in configuration register */
static int nand_std_enable_buf(flash_dev_t *dev)
{
    int ret;
    uint8_t status;

    ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
    CHECK_RET(ret);

    status |= FEAT_BUF_EN;

    ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
    CHECK_RET(ret);

    return FLASH_OK;
}

/*Clear BUF in configuration register*/
// static int nand_std_disable_buf(flash_dev_t *dev)
// {
//     int ret;
//     uint8_t status;

//     ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
//     CHECK_RET(ret);

//     status &= (~FEAT_BUF_EN);

//     ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
//     CHECK_RET(ret);

//     return FLASH_OK;
// }


static int nand_std_is_ecc_good(flash_dev_t *dev)
{
    int ret;
    uint8_t status;

    ret = nand_std_read_status(dev, FEATURE_STAT_ADDR, &status);
    CHECK_RET(ret);

    if ((status & STAT_ECC_MASK) >= STAT_ECC_UNCORR) {
        return -FLASH_ERROR_STATUS;
    }
    
    return FLASH_OK;
}

static int nand_std_enable_quad(flash_dev_t *dev)
{
    int ret;
    uint8_t status;

    ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
    CHECK_RET(ret);

    status |= FEAT_QUAD_EN;

    ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
    CHECK_RET(ret);

    return FLASH_OK;
}

static int nand_std_disable_quad(flash_dev_t *dev)
{
    int ret;
    uint8_t status;

    ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
    CHECK_RET(ret);

    status &= (~FEAT_QUAD_EN);

    ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
    CHECK_RET(ret);

    return FLASH_OK;
}

int nand_page_read(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *rx_buf, int rx_len, int check_ecc, read_mode_t readmode)
{
    (void)readmode;
    int ret;
    uint16_t page_addr, col_addr;
    uint8_t cmd[4];
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    bblk_lut_t *bblk_lut = (bblk_lut_t*)(&priv_params->bblk_lut);

    if (nand_bblk_lut_get(bblk_lut, blk) != 0) {
        LOGE(__func__, "bad blk %d", blk);
        return -FLASH_ERROR_STATUS;
    }
  
    if (priv_params->feature & NAND_ECC_STAT_SUP) {
        if (check_ecc) {
            ret = nand_std_enable_ecc(dev);
            CHECK_RET(ret);
        }
        else {
            ret = nand_std_disable_ecc(dev);
            CHECK_RET(ret);
        }
    }

    page_addr = GET_PAGE_ADDR(blk, page);

    cmd[0] = FLASH_PREAD;
    cmd[1] = 0; // dummy byte
    cmd[2] = (page_addr >> 8) & 0xff;
    cmd[3] = page_addr & 0xff;

    // PAGE READ (13h), transfer data from NAND flash array to cache
    ret = nand_std_send_data(dev, cmd, 4, 0, 0);
    CHECK_RET(ret);

    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    if ((priv_params->feature & NAND_ECC_STAT_SUP) && check_ecc) {
        ret = nand_std_is_ecc_good(dev);
        if (ret < 0) {
            LOGE(__func__, "ECC error: blk %d, page %d.", blk, page);
            nand_bblk_lut_set(bblk_lut, blk, 1);
            return ret;
        }
    }

    col_addr = GET_COL_ADDR(page_offset);
    
    cmd[0] = FLASH_READ;
    cmd[1] = (col_addr >> 8) & 0xff;
    cmd[2] = col_addr & 0xff;
    cmd[3] = 0; // dummy byte
    
    // READ (03h), read from cache
    ret = nand_std_receive_data(dev, cmd, 4, rx_buf, rx_len);
    CHECK_RET(ret);

    return FLASH_OK;
}

int nand_std_mark_bad_blk(flash_dev_t *dev, uint16_t blk, uint32_t badblockmarker)
{
    int ret;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);

    ret = nand_std_page_program(dev, blk, 0, priv_params->page_size, (uint8_t *)&badblockmarker, 4);

    return ret;
}

int nand_std_update_cfg_register(flash_dev_t *dev, uint8_t new_status, uint8_t mask)
{
    int ret;
    uint8_t status;
    ret = nand_std_read_status(dev, FEATURE_FEAT_ADDR, &status);
    CHECK_RET(ret);
    if ( (status & mask) != (new_status & mask) ) {
        status &= (~mask); // clear bits
        status |= (new_status & mask); // set bits with new status
        ret = nand_std_write_status(dev, FEATURE_FEAT_ADDR, status);
        CHECK_RET(ret);
    }

    return 0;
}

int nand_quad_page_read(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *rx_buf, int rx_len, int check_ecc, read_mode_t readmode)
{
    int ret;
    uint16_t page_addr, col_addr;
    uint8_t cmd[4];
    uint32_t read_cmd[2];
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    bblk_lut_t *bblk_lut = (bblk_lut_t*)(&priv_params->bblk_lut);

    if (nand_bblk_lut_get(bblk_lut, blk) != 0) {
        LOGE(__func__, "bad blk %d", blk);
        return -FLASH_ERROR_STATUS;
    }

    uint8_t mask = FEAT_ECC_EN | FEAT_BUF_EN;
    uint8_t status = 0;
    if (check_ecc) {
        status |= FEAT_ECC_EN;
    }
    if (readmode == BUFFER_MODE) {
        status |= FEAT_BUF_EN;
    }
    ret = nand_std_update_cfg_register(dev, status, mask);
    CHECK_RET(ret);

    page_addr = GET_PAGE_ADDR(blk, page);

    cmd[0] = FLASH_PREAD;
    cmd[1] = 0; // dummy byte
    cmd[2] = (page_addr >> 8) & 0xff;
    cmd[3] = page_addr & 0xff;

    // PAGE READ (13h), transfer data from NAND flash array to cache
    ret = nand_std_send_data(dev, cmd, 4, 0, 0);
    CHECK_RET(ret);

    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    if (check_ecc) {
        ret = nand_std_is_ecc_good(dev);
        if (ret < 0) {
            LOGE(__func__, "ECC error: blk %d, page %d.", blk, page);
            nand_bblk_lut_set(bblk_lut, blk, 1);
            nand_std_mark_bad_blk(dev, blk, FLASH_BAD_BLOCK_MARKER);
            return ret;
        }
    }

    col_addr = GET_COL_ADDR(page_offset);
    read_cmd[0] = FLASH_QREAD_FAST;
    read_cmd[1] = col_addr;


    // Quad READ (EBh), read from cache
    ret = nand_std_receive_data_enhanced(dev, read_cmd, 2, rx_buf, rx_len, readmode);
    CHECK_RET(ret);

    return FLASH_OK;
}

// Read the 1st byte of the 64-byte spare area in the 1st page and 2nd page.
// Non-FFh data indicates bad block.
static int nand_std_is_blk_good(flash_dev_t *dev, uint16_t blk)
{
    int ret;
    uint8_t byte0, byte1;
    int check_ecc;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    
    // 1. disable ecc to read the 64-byte extra data of the 1st page and 2nd page
    check_ecc = 0;
    ret = nand_std_page_read(dev, blk, 0, priv_params->page_size, &byte0, 1, check_ecc, BUFFER_MODE);
    CHECK_RET(ret);
    ret = nand_std_page_read(dev, blk, 1, priv_params->page_size, &byte1, 1, check_ecc, BUFFER_MODE);
    CHECK_RET(ret);
    
    // 2. Non-FFh checks.
    if ((byte0 != 0xff) || (byte1 != 0xff)) {
        LOGE(__func__, "blk(%d) is bad.", blk);
        return -FLASH_ERROR_STATUS;
    }
    
    return FLASH_OK;
}

/*int nand_flash_blk_erase(uint16_t blk)*/
int nand_std_blk_erase(flash_dev_t *dev, uint32_t addr, int force_erase)
{
    int ret;
    uint8_t cmd[4];
    uint16_t page_addr;
    uint8_t status;
    uint16_t blk;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    bblk_lut_t *bblk_lut = (bblk_lut_t*)(&priv_params->bblk_lut);
    uint32_t die_size = priv_params->die_size;

    if (priv_params->feature & NAND_MULT_DIE_SUP) {
        uint8_t die_num = addr / die_size;
        if (die_num != priv_params->current_die_index) {
            /*LOGI(__func__, "cur die num %d, expected die num %d, addr 0x%x", m_w25nxx_cur_die, die_num, addr);*/
            ret = nand_std_select_die(dev, die_num);
            CHECK_RET(ret);
        }
    }

    blk = addr / priv_params->block_size;
    /*ret = nand_std_is_blk_good(blk);*/
    /*CHECK_RET(ret);*/
    if (!force_erase) {
        if (nand_bblk_lut_get(bblk_lut, blk) != 0) {
            LOGE(__func__, "bad blk %d", blk);
            return -FLASH_ERROR_STATUS;
        }
    }
    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    ret = nand_std_write_enable(dev);
    CHECK_RET(ret);

    page_addr = GET_PAGE_ADDR(blk, 0);
    cmd[0] = FLASH_BERASE;
    cmd[1] = 0; // dummy byte
    cmd[2] = (page_addr >> 8) & 0xff;
    cmd[3] = page_addr & 0xff;
    
    ret = nand_std_send_data(dev, cmd, 4, 0, 0);
    CHECK_RET(ret);

    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    ret = nand_std_read_status(dev, FEATURE_STAT_ADDR, &status);
    CHECK_RET(ret);

    if (status & STAT_EFAIL) {
        LOGE(__func__, "Erase fail: blk %d.", blk);
        nand_bblk_lut_set(bblk_lut, blk, 1);
        nand_std_mark_bad_blk(dev, addr, FLASH_BAD_BLOCK_MARKER);
        return -FLASH_ERROR_ERASE;
    }
    nand_bblk_lut_set(bblk_lut, blk, 0);

    return FLASH_OK;
}

int nand_page_program(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *tx_buf, int tx_len)
{
    int ret;
    uint16_t page_addr, col_addr;
    uint8_t cmd[4];
    uint8_t status;

    if (tx_len == 0) {
        return FLASH_OK;
    }

    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    bblk_lut_t *bblk_lut = (bblk_lut_t*)(&priv_params->bblk_lut);

    if (nand_bblk_lut_get(bblk_lut, blk) != 0) {
        LOGE(__func__, "bad blk %d", blk);
        return -FLASH_ERROR_STATUS;
    }

    ret = nand_std_enable_ecc(dev);
    CHECK_RET(ret);

    ret = nand_std_write_enable(dev);
    CHECK_RET(ret);

    col_addr = GET_COL_ADDR(page_offset);
    cmd[0] = FLASH_PROG; // FLASH_PROG_RAN
    cmd[1] = (col_addr >> 8) & 0xff;
    cmd[2] = col_addr & 0xff;

    // program to cache (02h/84h)
    ret = nand_std_send_data(dev, cmd, 3, tx_buf, tx_len);
    CHECK_RET(ret);
    
    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    page_addr = GET_PAGE_ADDR(blk, page);
    cmd[0] = FLASH_PEXEC;
    cmd[1] = 0; // dummy byte
    cmd[2] = (page_addr >> 8) & 0xff;
    cmd[3] = page_addr & 0xff;

    ret = nand_std_write_enable(dev);
    CHECK_RET(ret);

    // execute program (10h)
    ret = nand_std_send_data(dev, cmd, 4, 0, 0);
    CHECK_RET(ret);
    
    ret = nand_std_read_status(dev, FEATURE_STAT_ADDR, &status);
    CHECK_RET(ret);

    if (status & STAT_PFAIL) {
        LOGE(__func__, "Flash program fail: blk %d, page %d.",
                blk, page);
        nand_bblk_lut_set(bblk_lut, blk, 1);
        return -FLASH_ERROR_WRITE;
    }

    return FLASH_OK;
}

int nand_quad_page_program(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *tx_buf, int tx_len)
{
    int ret;
    uint16_t page_addr, col_addr;
    uint8_t cmd[4];
    uint32_t prog_cmd[2];
    uint8_t status;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);

    if (tx_len == 0) {
        return FLASH_OK;
    }

    if (nand_bblk_lut_get(&priv_params->bblk_lut, blk) != 0) {
        LOGE(__func__, "bad blk %d", blk);
        return -FLASH_ERROR_STATUS;
    }

    ret = nand_std_enable_ecc(dev);
    CHECK_RET(ret);

    ret = nand_std_write_enable(dev);
    CHECK_RET(ret);

    col_addr    = GET_COL_ADDR(page_offset);
    prog_cmd[0] = FLASH_QPROG;
    prog_cmd[1] = col_addr;

    // program to cache (32h/34h)
    ret = nand_std_send_data_enhanced(dev, prog_cmd, 2, tx_buf, tx_len);
    CHECK_RET(ret);

    page_addr = GET_PAGE_ADDR(blk, page);
    cmd[0] = FLASH_PEXEC;
    cmd[1] = 0; // dummy byte
    cmd[2] = (page_addr >> 8) & 0xff;
    cmd[3] = page_addr & 0xff;

    // execute program (10h)
    ret = nand_std_send_data(dev, cmd, 4, 0, 0);
    CHECK_RET(ret);

    ret = nand_std_wait_for_ready(dev);
    CHECK_RET(ret);

    ret = nand_std_read_status(dev, FEATURE_STAT_ADDR, &status);
    CHECK_RET(ret);

    if (status & STAT_PFAIL) {
        nand_bblk_lut_set(&priv_params->bblk_lut, blk, 1);
        nand_std_mark_bad_blk(dev, blk, FLASH_BAD_BLOCK_MARKER);
        return -FLASH_ERROR_WRITE;
    }

    return FLASH_OK;
}

int nand_std_page_program(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *tx_buf, int tx_len)
{
    int ret;
    uint32_t addr;
    uint8_t  die_num;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t block_size = priv_params->block_size;
    uint32_t page_size = priv_params->page_size;

    // page program must not cross page
    configASSERT(tx_len <= (int)(page_size + priv_params->extra_size - page_offset));

    if (priv_params->feature & NAND_MULT_DIE_SUP) {
        addr = (uint32_t)blk * block_size + (uint32_t)page * page_size + page_offset;
        die_num = addr / priv_params->die_size;
        if (die_num != priv_params->current_die_index) {
            /*LOGI(__func__, "cur die num %d, expected die num %d, addr 0x%x", m_w25nxx_cur_die, die_num, addr);*/
            ret = nand_std_select_die(dev, die_num);
            CHECK_RET(ret);
        }
    }

    nand_page_program_func_t program_func = priv_params->quad_mode ? nand_quad_page_program : nand_page_program;

    ret = program_func(dev, blk, page, page_offset, tx_buf, tx_len);

    return ret;
}

int nand_std_blk_program(flash_dev_t *dev, uint32_t addr, uint8_t *tx_buf, uint32_t tx_len)
{
    uint16_t page;
    uint32_t tx_remain;
    int ret = FLASH_OK;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t block_size = priv_params->block_size;
    uint32_t page_size = priv_params->page_size;
    uint16_t blk = addr / block_size;

    uint16_t page_cnt = tx_len / page_size;

    tx_remain = tx_len & (page_size - 1);

    configASSERT(page_cnt <= block_size / page_size);

    for (page = 0; page < page_cnt; page++) {
        ret = nand_std_page_program(dev, blk, page, 0, tx_buf, page_size);
        if (ret < 0) {
            break;
        }
        tx_buf += page_size;
    }
    if (tx_remain) {
        ret = nand_std_page_program(dev, blk, page, 0, tx_buf, tx_remain);
    }

    return ret;
}

int nand_std_read_id(flash_dev_t *dev, uint8_t *manuf_id, uint16_t *device_id)
{
    uint8_t cmd[2] = {FLASH_RDID, 0};
    uint8_t data[3] = {0};
    int ret;

    ret = nand_std_receive_data(dev, cmd, 2, data, 3);
    if (ret < 0) {
        return ret;
    }
    *manuf_id = data[0];
    if (data[0] == 0xEF) { //winbond
        *device_id = ((uint16_t)(data[1]) << 8) | (uint16_t)(data[2]); 
    }
    else {
        *device_id = (uint16_t)(data[1]);
    }
    return FLASH_OK;
}

int nand_std_read_unique_id(flash_dev_t *dev, flash_unique_id_t *unique_id)
{
    (void)dev;
    (void)unique_id;

    LOGE(__func__, "Unsupported now!");

    return -FLASH_UNSPPORT;
}

static void get_blk_page_and_offset(flash_dev_t *dev, uint32_t addr, 
        uint16_t *blk_ptr,
        uint16_t *page_ptr,
        uint16_t *page_off_ptr
        )
{
    uint32_t blk, page;
    uint32_t blk_remain, page_remain;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t block_size = priv_params->block_size;
    uint32_t page_size = priv_params->page_size;

    blk  = addr / block_size;
    blk_remain = addr & (block_size - 1);
    page = blk_remain / page_size;
    page_remain = blk_remain & (page_size - 1);

    *blk_ptr  = blk & 0xffff;
    *page_ptr = page & 0xffff;
    *page_off_ptr = page_remain & 0xffff;
}

static read_mode_t calc_readmode(flash_dev_t *dev, uint32_t addr, uint32_t length)
{
    read_mode_t readmode = BUFFER_MODE;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t page_size = priv_params->page_size;
    // FIXME: disable continous read temperately.
#if 1
    if ((addr % page_size == 0) && (length > page_size)) {
        readmode = CONTINUOUS_MODE;
    }
    else {
        readmode = BUFFER_MODE;
    }
#else
        readmode = BUFFER_MODE;
#endif
    return readmode;
}

static int nand_std_select_die(flash_dev_t *dev, uint8_t die_num)
{
    int ret;
    uint8_t cmd[2];

    cmd[0] = FLASH_SELECT_DIE;
    cmd[1] = die_num;

    ret = nand_std_send_data(dev, cmd, 2, NULL, 0);
    
    if (ret == 0) {
        /*LOGI(__func__, "select die %d suc!", die_num);*/
        nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
        priv_params->current_die_index = die_num;
    }

    return ret;
}

int nand_std_read(flash_dev_t *dev, uint32_t addr, uint8_t *data_buf, uint32_t length)
{
    int ret = 0;
    uint32_t len = 0;
    uint16_t blk, page, page_offset;
    uint16_t page_remain;
    uint32_t die_remain;
    read_mode_t readmode = BUFFER_MODE;
    int check_ecc;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t chip_size = dev->chip_size;
    uint32_t page_size = priv_params->page_size;
    uint32_t die_size = priv_params->die_size;

    // 1. Check whether the addr is legal
    if (addr >= chip_size) {
        return -FLASH_WRONG_PARAM;
    }
    // 2. Make sure the length is no longer than the max flash size
    length = (addr + length) > chip_size ? (chip_size - addr) : length;
    
    blk  = 0;
    page = 0;
    page_offset = 0;

    check_ecc = 1;
    while (length) {
        readmode = calc_readmode(dev, addr, length);
        get_blk_page_and_offset(dev, addr, &blk, &page, &page_offset);
        if (readmode == BUFFER_MODE) {
            page_remain = page_size - page_offset;
            len = (length >= page_remain) ? page_remain : length;
            ret = nand_std_page_read(dev, blk, page, page_offset, data_buf, len, check_ecc, readmode);
        }
        else if (readmode == CONTINUOUS_MODE) {
            len = (length >= FIFO_NAND_LEN) ? FIFO_NAND_LEN : length;
            // make sure read will not cross die, buffer mode has make sure not cross page,
            // so will not cross die
            if ((priv_params->feature & NAND_MULT_DIE_SUP)) {
                die_remain = die_size - (addr % die_size);
                len = (len >= die_remain) ? die_remain : len;
            }
            ret = nand_std_page_read(dev, blk, page, page_offset, data_buf, len, check_ecc, readmode);
        }
        else {
            ret = -FLASH_UNSPPORT;
            break;
        }

        if (ret < 0) {
            return ret;
        }
        addr     += len;
        data_buf += len;
        length   -= len;
    }

    // switch back to buffer mode
    if (readmode == CONTINUOUS_MODE) {
        uint8_t mask   = FEAT_ECC_EN | FEAT_BUF_EN;
        uint8_t status = FEAT_ECC_EN | FEAT_BUF_EN;
        ret = nand_std_update_cfg_register(dev, status, mask);
    }

    return ret;
}


int nand_std_page_read(flash_dev_t *dev, uint16_t blk, uint16_t page, uint16_t page_offset, 
        uint8_t *rx_buf, int rx_len, int check_ecc, read_mode_t readmode)
{
    int ret;
    uint32_t addr;
    uint8_t  die_num;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t block_size = priv_params->block_size;
    uint32_t page_size = priv_params->page_size;
    uint32_t die_size = priv_params->die_size;

    if (priv_params->feature & NAND_MULT_DIE_SUP) {
        addr = (uint32_t)blk * block_size + (uint32_t)page * page_size + page_offset;
        die_num = addr / die_size;
        if (die_num != priv_params->current_die_index) {
            /*LOGI(__func__, "cur die num %d, expected die num %d, addr 0x%x", m_w25nxx_cur_die, die_num, addr);*/
            ret = nand_std_select_die(dev, die_num);
            CHECK_RET(ret);
        }
    }

    nand_read_page_func_t read_func = priv_params->quad_mode ? nand_quad_page_read : nand_page_read;
    ret = read_func(dev, blk, page, page_offset, rx_buf, rx_len, check_ecc, readmode);

    return ret;
}

int nand_std_write(flash_dev_t *dev, uint32_t addr, uint8_t *data_buf, uint32_t length)
{
    int ret = FLASH_OK;
    uint32_t blk_addr = 0;
    uint32_t blk_offset = 0;
    uint32_t blk_remain = 0;
    uint32_t write_len = 0;
    uint32_t index = 0;
    uint8_t *pread = NULL;
    uint8_t *pwrite = NULL;
    uint8_t *src_buf = data_buf;
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    uint32_t chip_size = dev->chip_size;
    uint32_t block_size = priv_params->block_size;

    uint8_t *swap_buf = aiva_malloc(block_size);
    
    configASSERT(swap_buf != NULL);

    // 1. Check whether the addr is legal
    if (addr >= chip_size) {
        ret = -FLASH_WRONG_PARAM;
        goto out;
    }
    // 2. Make sure the length is no longer than the max flash size
    length = (addr + length) > chip_size ? (chip_size - addr) : length;

    while (length)
    {
        blk_addr   = addr & (~(block_size - 1));
        blk_offset = addr & (block_size - 1);
        blk_remain = block_size - blk_offset;

        write_len = ((length < blk_remain) ? length : blk_remain);

        if (write_len == block_size && addr == blk_addr) {
            src_buf = data_buf;
        }
        else {
            if (!swap_buf) {
                swap_buf = aiva_malloc(block_size);
                configASSERT(swap_buf != NULL);
            }
            ret = dev->ops->read(dev, blk_addr, swap_buf, block_size);
            // if (ret < 0) {
            //     goto out;
            // }
            pread = swap_buf + blk_offset;
            pwrite = data_buf;
            for (index = 0; index < write_len; index++)
                *pread++ = *pwrite++;
            src_buf = swap_buf;
        }

        ret = nand_std_blk_erase(dev, blk_addr, NAND_FORCE_ERASE);
        if (ret < 0) {
            goto out;
        }

        ret = nand_std_blk_program(dev, blk_addr, src_buf, block_size);
        if (ret < 0) {
            goto out;
        }
        length   -= write_len;
        addr     += write_len;
        data_buf += write_len;
    }
out:
    if (swap_buf) {
        aiva_free(swap_buf);
    }
    return ret;
}


int nand_std_init(flash_dev_t *dev, const flash_dev_config_t *cfg)
{
    int ret = 0;
    uint8_t manuf_id;
    uint16_t device_id;
    int max_blk;

    dev->cfg = *cfg;

    spi_device_num_t spi_num = cfg->spi_num;


    spi_init(spi_num, SPI_WORK_MODE_0, SPI_FF_STANDARD, FIFO_W8);
    spi_set_clk_rate(spi_num, cfg->clk_rate);

    ret = nand_std_read_id(dev, &manuf_id, &device_id);
    CHECK_RET(ret);

    if (dev->manuf_id == 0) {
        dev->manuf_id = manuf_id;
    }

    if (dev->device_id == 0) {
        dev->device_id = device_id;
    }

    ret = nand_std_reset(dev);
    CHECK_RET(ret);

    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    priv_params->quad_mode = (uint8_t)cfg->enable_quad_rw;
    if (priv_params->quad_mode_should_write_regs) {
        if (priv_params->quad_mode)
            nand_std_enable_quad(dev);
        else
            nand_std_disable_quad(dev);
    }

    if (priv_params->feature & NAND_BUF_STAT_SUP) {
        ret = nand_std_enable_buf(dev);
        CHECK_RET(ret);
    }
    max_blk = dev->chip_size / priv_params->block_size;
    priv_params->bblk_lut.lut = aiva_calloc(max_blk, 1);
    configASSERT(priv_params->bblk_lut.lut != NULL);
    priv_params->bblk_lut.max_blk = max_blk;

    nand_bblk_lut_init(&priv_params->bblk_lut, nand_std_is_blk_good);

    if (priv_params->feature & NAND_MULT_DIE_SUP) {
        ret = nand_std_select_die(dev, 0);
    }

    return ret;
}

int nand_std_release(flash_dev_t *dev)
{
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    bblk_lut_t *bblk_lut = (bblk_lut_t*)(&priv_params->bblk_lut);
    if (bblk_lut->lut) {
        aiva_free(bblk_lut->lut);
        bblk_lut->lut = NULL;
    }

    return 0;
}

int nand_std_get_param(flash_dev_t *dev, flash_param_t *pparam)
{
    *pparam = FLASH_PARAM_INITIALIZER;

    pparam->dev_name = dev->name;
    pparam->mid = dev->manuf_id;
    pparam->did = dev->device_id;
    pparam->chip_size   = dev->chip_size;
    //
    nand_flash_priv_params_t *priv_params = (nand_flash_priv_params_t*)(dev->priv);
    pparam->page_size   = priv_params->page_size;
    pparam->block_size = priv_params->block_size;
    pparam->feature = priv_params->feature;

    return 0;
}

static flash_dev_ops_t m_flash_ops = {
    .init = nand_std_init,
    .read = nand_std_read,
    .write = nand_std_write,
    .erase = nand_std_blk_erase,
    .program = nand_std_blk_program,
    .release = nand_std_release,
    .read_unique_id = nand_std_read_unique_id,
    .get_param = nand_std_get_param,
    .lock = NULL,
    .unlock = NULL,
};

static nand_flash_priv_params_t m_nand_flash_priv_params[] = {
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = NAND_ECC_STAT_SUP, .quad_mode = 0, .quad_mode_should_write_regs = 0},
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = NAND_ECC_STAT_SUP, .quad_mode = 0, .quad_mode_should_write_regs = 1},
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = 0, .quad_mode = 0, .quad_mode_should_write_regs = 1},
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = NAND_ECC_STAT_SUP | NAND_BUF_STAT_SUP, .quad_mode = 0, .quad_mode_should_write_regs = 0},
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = NAND_ECC_STAT_SUP | NAND_BUF_STAT_SUP, .quad_mode = 0, .quad_mode_should_write_regs = 0},
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = NAND_ECC_STAT_SUP, .quad_mode = 0, .quad_mode_should_write_regs = 1},
    {.block_size = 128*1024, .page_size = 2048, .extra_size = 64, .die_size = 128*1024*1024, .current_die_index = 0, .feature = NAND_ECC_STAT_SUP, .quad_mode = 0, .quad_mode_should_write_regs = 1},
};

// NOTE: mid did is set to 0 to mark update when init
// NOTE: "nand_std" is used by flash_dev to search for default nor flash driver
static flash_dev_t nand_std_devices[] = {
    [0] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0x00, 0x0000, "nand_std", 128*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[0]),
    [1] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0xc2, 0x0012, "mx35lf1g", 128*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[1]),
    [2] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0xc2, 0x0022, "mx35lf2g", 256*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[2]),
    [3] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0xef, 0xaa21, "w25n01gv", 128*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[3]),
    [4] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0xef, 0xab21, "w25m02gv", 256*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[4]),
    [5] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0xc8, 0x00f1, "gd5f1gq4", 128*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[5]),
    [6] = FLASH_DEV_INITIALIZER_WITH_PARAMS(FLASH_NAND, 0xc8, 0x00c9, "gd5f1gq4", 128*1024*1024, &m_flash_ops, 0, &m_nand_flash_priv_params[6]),
};

/* Export Device:
 * 	    [uclass ID], [device name], [flags], [driver], [private pointer]
 */
UDEVICE_EXPORT(UCLASS_SPI_FLASH,     nand_std,     0,      &nand_std_devices[0],    NULL);
