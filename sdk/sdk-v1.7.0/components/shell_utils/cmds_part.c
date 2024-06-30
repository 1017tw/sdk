#include <string.h>
#include <stdlib.h>
#include "syslog.h"
#include "flash_part.h"
#include "boot_flash.h"
#include "spi.h"
#include "shell.h"
#include "FreeRTOS.h"
#include "os_init_app.h"
#include "fs_file.h"

#define SPI_BOOT_DEV        SPI_DEVICE_0
#define SPI_BAUD            (8)

static void partition_create_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "Usage: part_create [name] [offset] [size]\n");
    shellPrint(shell, "  Create one partition: name - string, size - uint32_t\n");
}

static void partition_update_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "Usage: part_update [name] [offset] [size]\n");
    shellPrint(shell, "  Update one partition: name - string, offset - uint32_t\n");
}

static void partition_list_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "Usage: part_list, list all partitions\n");
}

static void partition_del_all_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "Usage: part_del_all, delete all partitions\n");
}

int get_mask_flags(uint32_t *mask_flags_ptr)
{
    uint32_t mask_flags;
    int ret;
    int flash_type;

    flash_type = os_get_flash_type();

    ret = 0;
    if (flash_type == FLASH_NOR) {
        mask_flags = MASK_FLAG_NOR_FLASH;
    }
    else if (flash_type == FLASH_NAND) {
        mask_flags = MASK_FLAG_NAND_FLASH;
    }
    else {
        ret = -1;
        mask_flags = 0;
    }

    *mask_flags_ptr = mask_flags;

    return ret;
}

int cons_cmd_partition_create(int argc, char *argv[])
{
    uint32_t offset, size;
    uint32_t mask_flags;
    char *name;
    int ret;
    Shell *shell = shellGetCurrent();

    if (argc != 4) {
        partition_create_help();
        return -1;
    }

    name   = argv[1];
    offset = strtoul(argv[2], NULL, 0);
    size   = strtoul(argv[3], NULL, 0);

    ret = get_mask_flags(&mask_flags);
    if (ret < 0) {
        partition_create_help();
        return -1;
    }

    // NOTE: lock fs opertion before changing partition table
    fs_get_lock();
    ret = flash_part_create(name, offset, size, mask_flags);
    fs_release_lock();

    if (ret < 0) {
        fs_get_lock();
        ret = flash_part_update(name, offset, size, mask_flags);
        fs_release_lock();
    }

    if (ret == 0) {
        shellPrint(shell, "Create partition '%s' success!\n", name);
    }

    return ret;
}

int cons_cmd_partition_update(int argc, char *argv[])
{
    uint32_t offset, size;
    uint32_t mask_flags;
    char *name;
    int ret;

    if (argc != 4) {
        partition_update_help();
        return -1;
    }

    name   = argv[1];
    offset = strtoul(argv[2], NULL, 0);
    size   = strtoul(argv[3], NULL, 0);
    
    ret = get_mask_flags(&mask_flags);
    if (ret < 0) {
        partition_update_help();
        return -1;
    }

    // NOTE: lock fs opertion before changing partition table
    fs_get_lock();
    ret = flash_part_update(name, offset, size, mask_flags);
    fs_release_lock();

    return ret;
}

int cons_cmd_partition_list(int argc, char *argv[])
{
    (void)(argv);

    uint32_t mask_flags;
    int ret;

    if (argc != 1) {
        partition_list_help();
        return -1;
    }

    ret = get_mask_flags(&mask_flags);
    if (ret < 0) {
        partition_list_help();
        return -1;
    }


    fs_get_lock();
    ret = flash_part_list(mask_flags);
    fs_release_lock();

    return ret;
}

int cons_cmd_partition_del_all(int argc, char *argv[])
{
    (void)(argv);

    uint32_t mask_flags;
    int ret;

    if (argc != 1) {
        partition_del_all_help();
        return -1;
    }
    
    ret = get_mask_flags(&mask_flags);
    if (ret < 0) {
        partition_del_all_help();
        return -1;
    }

    fs_get_lock();
    ret = flash_part_delete_all(mask_flags);
    fs_release_lock();

    return ret;
}

/*# 0x00000000 -------> +-------------------------------------+*/
/*#                     |     SPL           (63KB)            |*/
/*# 0x0000FC00 -------> +-------------------------------------+*/
/*#                     |     Part Table0   (1KB)             |*/
/*# 0x00010000 -------> +-------------------------------------+*/
/*#                     |     FW image0     (852KB)           |*/
/*# 0x000E5000 -------> +-------------------------------------+*/
/*#                     |     FW image1     (360KB)           |*/
/*# 0x0013F000 -------> +-------------------------------------+*/
/*#                     |     XNN weights   (6MB+516KB)       |*/
/*# 0x007C0000 -------> +-------------------------------------+*/
/*#                     |     face_db       (128KB)           |*/
/*# 0x007E0000 -------> +-------------------------------------+*/
/*#                     |     File system   (128KB)           |*/
/*# 0x007FFFFF -------> +-------------------------------------+*/


int cons_cmd_partition_default(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int ret;
    uint32_t mask_flags;

#define FW_SIZE_0           (852*1024) 		                /* 852KB */
#define FW_SIZE_1           (360*1024) 		                /* 360KB */
#define XNN_WGT_SIZE        (6*1024*1024 + 516*1024) 		/* 6MB + 516KB */
#define FACE_DB_SIZE 		(128 * 1024) 		            /* 128KB */
#define FS_SIZE 			(128 * 1024) 		            /* 128KB */
#define NOR_FLASH_SIZE      (8*1024*1024)
#define NAND_FLASH_SIZE     (128*1024*1024)

#define FW_ADDR_0           (0x00010000)                    /* SPL(63 KB) + (1KB) */
#define FW_ADDR_1           (FW_ADDR_0 + FW_SIZE_0)
#define XNN_WGT_ADDR        (FW_ADDR_1 + FW_SIZE_1)
#define FACE_DB_ADDR 		(XNN_WGT_ADDR + XNN_WGT_SIZE)
#define FS_ADDR             (FACE_DB_ADDR + FACE_DB_SIZE)

    ret = get_mask_flags(&mask_flags);
    if (ret < 0) {
        return ret;
    }
    // name, offset, size, mask_flags
    ret = flash_part_create("fw0", FW_ADDR_0, FW_SIZE_0, mask_flags);
    if (ret < 0) {
        return ret;
    }
    
    ret = flash_part_create("fw1", FW_ADDR_1, FW_SIZE_1, mask_flags);
    if (ret < 0) {
        return ret;
    }

    ret = flash_part_create("xnn_wgt", XNN_WGT_ADDR, XNN_WGT_SIZE, mask_flags);
    if (ret < 0) {
        return ret;
    }
    
    ret = flash_part_create("face_db", FACE_DB_ADDR, FACE_DB_SIZE, mask_flags);
    if (ret < 0) {
        return ret;
    }

    if (mask_flags & MASK_FLAG_NOR_FLASH) {
        ret = flash_part_create("fs", FS_ADDR, NOR_FLASH_SIZE-FS_ADDR, mask_flags);
    }
    else {
        ret = flash_part_create("fs", FS_ADDR, NAND_FLASH_SIZE-FS_ADDR, mask_flags);
    }

    return ret;
}

void cons_cmd_part_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "Usage: part [OPTIONS] [ARGS] \r\n"
                "   [OPTIONS]: \r\n"
                "      - create : create a partition,       ARGS: [name], [offset], [size] \r\n"
                "      - update : update a partition,       ARGS: [name], [offset], [size] \r\n"
                "      - del_all: delete all partitions,    ARGS: none\r\n"
                "      - ls     : list all partions,        ARGS: none\r\n"
                "      - default: init default part tables, ARGS: none\r\n"
            );
}

int cons_cmd_part(int argc, char *argv[])
{
    if (argc < 2) {
        cons_cmd_part_help();
        return -1;
    }

    if (strncmp(argv[1], "create", 6) == 0) {
        cons_cmd_partition_create(argc-1, &argv[1]);
    }
    else if (strncmp(argv[1], "update", 6) == 0) {
        cons_cmd_partition_update(argc-1, &argv[1]);
    }
    else if (strncmp(argv[1], "del_all", 7) == 0) {
        cons_cmd_partition_del_all(argc-1, &argv[1]);
    }
    else if (strncmp(argv[1], "ls", 2) == 0) {
        cons_cmd_partition_list(argc-1, &argv[1]);
    }
    else if (strncmp(argv[1], "default", 7) == 0) {
        cons_cmd_partition_default(argc-1, &argv[1]);
    }
    else{
        cons_cmd_part_help();
    }

    return 0;
}


/* _attr, _name, _func, _desc */
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
part, cons_cmd_part, Partition operations);
