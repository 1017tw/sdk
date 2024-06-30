#include <stdint.h>
#include <stdbool.h>
#include "mem_AI3100.h"
#include "mem_ld.h"

enum sys_exec_mode_t {
    EXECUTE_MODE_ROM = 0,
    EXECUTE_MODE_DDR,
};

#if defined(EXECUTE_ON_ROM)
unsigned char __attribute__((section (".bootSection"))) g_exec_mode = EXECUTE_MODE_ROM;
#elif defined(EXECUTE_ON_DDR)
unsigned char __attribute__((section (".bootSection"))) g_exec_mode = EXECUTE_MODE_DDR;
#else
#   error Must define EXECUTE_ON_ROM or EXECUTE_ON_DDR mode!!!
#endif

bool is_xip_exec_mode(void)
{
    if (g_exec_mode != EXECUTE_MODE_ROM)
        return false;

    return true;
}

bool is_ddr_exec_mode(void)
{
    if (g_exec_mode != EXECUTE_MODE_DDR)
        return false;

    return true;
}

uint32_t get_ddr0_base(void)
{
    return __DDR0_BASE;
}

uint32_t get_ddr0_size(void)
{
    return __DDR0_SIZE;
}

uint32_t get_ddr1_base(void)
{
    return __DDR1_BASE;
}

uint32_t get_ddr1_size(void)
{
    return __DDR1_SIZE;
}

uint32_t get_ddr_heap_size(void)
{
    return __DDR_HEAP_SIZE;
}

uint32_t get_dma_mem_base(void)
{
    return __DMA_MEM_BASE;
}

uint32_t get_dma_mem_size(void)
{
    return __DMA_MEM_SIZE;
}

uint32_t get_ttb_base(void)
{
    return __TTB_BASE;
}

uint32_t get_ttb_size(void)
{
    return __TTB_SIZE;
}

uint32_t get_xnn_buf_ping_base(void)
{
    return __XNN_BUF_PING_BASE;
}

uint32_t get_xnn_buf_pong_base(void)
{
    return __XNN_BUF_PONG_BASE;
}

uint32_t get_xnn_buf_ping_size(void)
{
    return __XNN_BUF_PING_SIZE;
}

uint32_t get_xnn_buf_pong_size(void)
{
    return __XNN_BUF_PONG_SIZE;
}

uint32_t get_small_mem_size(void)
{
    return CONFIG_SMALL_MEM_SIZE_BYTE;
}
