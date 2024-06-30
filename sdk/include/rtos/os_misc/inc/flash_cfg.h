#ifndef _FLASH_CFG_H_
#define _FLASH_CFG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_FPGA
static const uint32_t NOR_FLASH_CLK_RATE  = 20*1000*1000;
static const uint32_t NAND_FLASH_CLK_RATE = 20*1000*1000;
#else
static const uint32_t NOR_FLASH_CLK_RATE  = 75*1000*1000;
static const uint32_t NAND_FLASH_CLK_RATE = 75*1000*1000;
#endif

#ifdef __cplusplus
}
#endif 

#endif //_FLASH_CFG_H_
