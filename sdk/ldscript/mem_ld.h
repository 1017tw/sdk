#ifndef __MEM_LD_H__
#define __MEM_LD_H__

#include "mem_AI3100.h"

/* NOTE: this file is specially designed for link script file, it MUST NOT be included in SDK files */

#define PRINT_MACRO_HELPER(x)  #x
#define PRINT_MACRO(x)         #x "=" PRINT_MACRO_HELPER(x)

/*#------------------- SRAM memory layout --------------------#*/
/*#                       [  256KB  ]                         #*/
/*# 0x10000000 -------> +-------------------------------------+*/
/*#                     |     TTB (16KB)                      |*/
/*# 0x10004000 -------> +-------------------------------------+*/
/*#                     |     TTB_L2 (4KB, 8KB)               |*/
/*# 0x10006000 -------> +-------------------------------------+*/ // ---.
/*#                     |     SRAM code/data(.text, .bss)     |*/ //    |
/*# 0x1000xx00 -------> +-------------------------------------+*/ //    |
/*#                     |     FW heap (small block)           |*/ //  SRAM_SEG_SIZE
/*# 0x1000xx00 -------> +-------------------------------------+*/ //    |
/*#                     |     FW stack                        |*/ //    |
/*# 0x10040000 -------> +-------------------------------------+*/ // ---'

// Non-XIP debug mode, 4MB-DDR memory layout
/*#------------------- DDR memory layout ---------------------#*/
/*#                       [  4MB DDR ]                        #*/
/*# 0x80800000 -------> +-------------------------------------+*/ // ---.
/*#                     |     FW code (non-xip mode)          |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ //  FW_SPACE_SIZE
/*#                     |     FW Heap (large block)           |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ // ---.
/*#                     |     DMA (.bss)                      |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ //  DMA_MEM_SIZE
/*#                     |     DMA (heap)                      |*/ //    |
/*# 0x80C00000 -------> +-------------------------------------+*/ // ---'

// Non-XIP debug mode, 8MB-DDR memory layout
/*#------------------- DDR memory layout ---------------------#*/
/*#                       [  8MB DDR  ]                       #*/
/*# 0x80400000 -------> +-------------------------------------+*/ // 
/*#                     |     XNN BUF-Ping                    |*/ //    
/*# 0x80600000 -------> +-------------------------------------+*/ // ---.
/*#                     |     FW code (non-xip mode)          |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ //  FW_SPACE_SIZE
/*#                     |     FW Heap (large block)           |*/ //    |
/*# 0x80A00000 -------> +-------------------------------------+*/ // ---'
/*#                     |     XNN BUF-Pong                    |*/ // 
/*# 0x80xxxxxx -------> +-------------------------------------+*/ // ---.
/*#                     |     DMA (.bss)                      |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ //  DMA_MEM_SIZE
/*#                     |     DMA (heap)                      |*/ //    |
/*# 0x80C00000 -------> +-------------------------------------+*/ // ---'

// XIP debug mode, 8MB-DDR memory layout
/*#------------------- DDR memory layout ---------------------#*/
/*#                       [  8MB DDR  ]                       #*/
/*# 0x80400000 -------> +-------------------------------------+*/ //
/*#                     |     ....                            |*/ //
/*# 0x804100A0 -------> +-------------------------------------+*/ // ---.
/*#                     |     FW code                         |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ //  FW_SPACE_SIZE
/*#                     |     FW Heap (large block)           |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ // ---.
/*#                     |     DMA (.bss)                      |*/ //    |
/*# 0x80xxxxxx -------> +-------------------------------------+*/ //  DMA_MEM_SIZE
/*#                     |     DMA (heap)                      |*/ //    |
/*# 0x80C00000 -------> +-------------------------------------+*/ // ---'

/* SRAM code/data segement base address */
#define __SRAM_SEG_BASE                         (__SRAM_BASE + __TTB_SIZE + __TTB_L2_SIZE)
#define __SRAM_SEG_SIZE                         (__SRAM_SIZE - __TTB_SIZE - __TTB_L2_SIZE)

/* TTB L2: table size */
/* NOTE: to support all chips in one sdk, set fix value of 8 ttb l2 */
#define __TTB_L2_SIZE                           (8 * __TABLE_L2_4K_SIZE)

// TODO: increase code size if it is not enough !!!
#define __SRAM_CODE_SIZE                        (CONFIG_SRAM_CODE_SIZE_KB * 1024)
#define __SRAM_HEAP_BASE                        (__SRAM_SEG_BASE + __SRAM_CODE_SIZE)
#define __SRAM_HEAP_SIZE                        (__SRAM_SIZE - __TTB_SIZE - __TTB_L2_SIZE - __SRAM_CODE_SIZE - __STACK_SIZE_ALL)


/* DDR0 */
#define __DDR0_BASE                             (0x80400000)
#define __DDR0_SIZE                             (CONFIG_DDR0_SIZE_MB * 1024 * 1024) /* Default: 4MB */

/* DDR1 */
#define __DDR1_BASE                             (0x80800000)
#define __DDR1_SIZE                             (CONFIG_DDR1_SIZE_MB * 1024 * 1024) /* Default: 4MB */

#if (__DDR0_SIZE != 0)
#   define __DDR_BASE                           __DDR0_BASE
#else
#   define __DDR_BASE                           __DDR1_BASE
#endif

#define __DDR_CACHE_REGION_SIZE                 (__DDR0_SIZE + __DDR1_SIZE - __DMA_MEM_SIZE)

#if (defined (XIP_EXEC_DEBUG)) && (!defined (EXECUTE_ON_ROM))
#   error Wrong config!!! Must define XIP_EXEC_DEBUG and EXECUTE_ON_ROM at the same time!
#endif

#if (defined (XNN_PING_PONG_BUF)) && ((defined (CONFIG_CHIP_AI1101A)) \
    || (defined (CONFIG_CHIP_AI1102A)) || (defined (CONFIG_CHIP_AI1103A))) 
# error "This chip can not support XNN_PING_PONG_BUF !!!"
#endif

#if (!defined (XIP_EXEC_DEBUG)) && defined (XNN_PING_PONG_BUF) 
#   define __XNN_BUF_PING_SIZE                  (1 * 1024 * 1024)
#   define __XNN_BUF_PONG_SIZE                  (2 * 1024 * 1024 - __DMA_MEM_SIZE)
#else
#   define __XNN_BUF_PING_SIZE                  (0)
#   define __XNN_BUF_PONG_SIZE                  (0)
#endif


/* stack offset from __SRAM_SEG_BASE */
#define __SRAM_STACK_OFFSET                     (__SRAM_CODE_SIZE + __SRAM_HEAP_SIZE)
#define __SRAM_STACK_BASE                       (__SRAM_SEG_BASE + __SRAM_STACK_OFFSET)

#define __STACK_SIZE_ALL (__UND_STACK_SIZE + __ABT_STACK_SIZE + __SVC_STACK_SIZE + __IRQ_STACK_SIZE + __FIQ_STACK_SIZE + __SYS_STACK_SIZE)


#ifdef XIP_EXEC_DEBUG
#   define __DDR_FW_SPACE_SIZE                  (__DDR1_SIZE - __DMA_MEM_SIZE)
#       define __DDR_FW_CODE_BASE               (__DDR1_BASE)
#       define __DDR_FW_CODE_SIZE               (0) 	/* 0KB for XIP DEBUG simulation */
#else
#   define __DDR_FW_SPACE_SIZE                  (__DDR_CACHE_REGION_SIZE - __XNN_BUF_PING_SIZE - __XNN_BUF_PONG_SIZE)
#       define __DDR_FW_CODE_BASE               (__DDR_BASE + __XNN_BUF_PING_SIZE)
#       define __DDR_FW_CODE_SIZE               (CONFIG_FW_CODE_SIZE_KB * 1024) 	/* Default: 1280 KB */
#endif

#define __SMALL_MEM_POOL_SIZE                   (CONFIG_SMALL_MEM_POOL_SIZE_BYTE)
#define __DDR_HEAP_BASE                         (__DDR_FW_CODE_BASE  + __DDR_FW_CODE_SIZE)
#define __DDR_HEAP_SIZE                         (__DDR_FW_SPACE_SIZE - __DDR_FW_CODE_SIZE)

//#define __DMA_MEM_BASE                       	(__DDR_BASE + __DDR_FW_SPACE_SIZE)
#define __DMA_MEM_BASE                       	(__DDR_FW_CODE_BASE + __DDR_FW_CODE_SIZE + __DDR_HEAP_SIZE + __XNN_BUF_PONG_SIZE)
#define __DMA_MEM_SIZE                       	(CONFIG_DMA_MEM_SIZE_KB  * 1024)
#	define __DMA_HEAP_SIZE                  	(CONFIG_DMA_HEAP_SIZE_KB * 1024)

//#define __DMA_PAGE_4K_CNT                       (__DMA_MEM_SIZE / (4*1024))

#define __XNN_BUF_PING_BASE                     (__DDR_BASE)
#define __XNN_BUF_PONG_BASE                     (__DDR1_BASE + __DDR1_SIZE - __XNN_BUF_PONG_SIZE - __DMA_MEM_SIZE)

#define XNN_BUF_PING_BASE_CHECK                 (__XNN_BUF_PING_BASE == __DDR_FW_CODE_BASE - __XNN_BUF_PING_SIZE)
#define XNN_BUF_PONG_BASE_CHECK                 (__XNN_BUF_PONG_BASE == __DDR_FW_CODE_BASE + __DDR_FW_CODE_SIZE + __DDR_HEAP_SIZE)

#define IS_DMA_MEMORY(a) ( ((a) >= __DMA_MEM_BASE) && ((a) < (__DMA_MEM_BASE+__DMA_MEM_SIZE)) )


#define FW_MEM_ASSERT (__DDR_FW_SPACE_SIZE == (__DDR_FW_CODE_SIZE + __DDR_HEAP_SIZE) \
                    && __DDR_FW_SPACE_SIZE > 0 \
                    && __DDR_FW_CODE_SIZE >= 0 \
                    && __DDR_HEAP_SIZE > 0 \
                    )

#define FW_SPACE_SIZE_ASSERT (__DDR_FW_SPACE_SIZE > 0)
#define FW_CODE_SIZE_ASSERT  (__DDR_FW_CODE_SIZE >= 0)
#define FW_HEAP_SIZE_ASSERT  (__DDR_HEAP_SIZE > 0)

#if (!(FW_SPACE_SIZE_ASSERT))
#error FW space size error!
#endif

#if (!(FW_CODE_SIZE_ASSERT))
#error FW code size error!
#endif

#if (!(FW_HEAP_SIZE_ASSERT))
#error FW heap size error!
#endif

#ifndef XIP_EXEC_DEBUG
#   if (!(XNN_BUF_PING_BASE_CHECK))
#       error XNN buf ping base address check error!!!
#   endif

#   if (!(XNN_BUF_PONG_BASE_CHECK))
#       error XNN buf pong base address check error!!!
#   endif
#endif


#define DDR_MEM_ASSERT (__DMA_MEM_SIZE > 0 \
                    &&  __DMA_HEAP_SIZE > 0 \
                    &&  __DMA_HEAP_SIZE < __DMA_MEM_SIZE)

//#pragma message(PRINT_MACRO(__DDR_CACHE_REGION_SIZE))
//#pragma message(PRINT_MACRO(__DDR_FW_CODE_BASE))
//#pragma message(PRINT_MACRO(__DDR_FW_SPACE_SIZE))
//#pragma message(PRINT_MACRO(__DDR_FW_CODE_SIZE))
//#pragma message(PRINT_MACRO(__DDR_HEAP_SIZE))
//#pragma message(PRINT_MACRO(__DMA_MEM_BASE))
//#pragma message(PRINT_MACRO(__DMA_MEM_SIZE))
//#pragma message(PRINT_MACRO(CONFIG_DMA_MEM_SIZE_KB))
//#pragma message(PRINT_MACRO(CONFIG_DMA_HEAP_SIZE_KB))
//#pragma message(PRINT_MACRO(__SRAM_SEG_SIZE))
//#pragma message(PRINT_MACRO(__SRAM_STACK_OFFSET))
//#pragma message(PRINT_MACRO(CONFIG_FW_CODE_SIZE_KB))
//#pragma message(PRINT_MACRO(__XNN_BUF_PING_BASE))
//#pragma message(PRINT_MACRO(__XNN_BUF_PONG_BASE))

#if (!(FW_MEM_ASSERT) || !(DDR_MEM_ASSERT))
#error !!! Wrong memory map !!!
#endif

#endif // __MEM_LD_H__
