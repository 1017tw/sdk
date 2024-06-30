#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "shell.h"
#include "syslog.h"
#include "mem_AI3100.h"

extern uint32_t                     Image$$SMALL_HEAP$$ZI$$Base;
extern uint32_t                     Image$$SMALL_HEAP$$ZI$$Limit;
#define SMALL_HEAP_START_ADDR       (&Image$$SMALL_HEAP$$ZI$$Base)
#define SMALL_HEAP_END_ADDR         (&Image$$SMALL_HEAP$$ZI$$Limit)

extern uint32_t                     Image$$HEAP$$ZI$$Base;
extern uint32_t                     Image$$HEAP$$ZI$$Limit;
#define HEAP_START_ADDR             (&Image$$HEAP$$ZI$$Base)
#define HEAP_END_ADDR               (&Image$$HEAP$$ZI$$Limit)

extern uint32_t                     Image$$DMA_HEAP$$ZI$$Base;
extern uint32_t                     Image$$DMA_HEAP$$ZI$$Limit;
#define DMA_HEAP_START_ADDR         (&Image$$DMA_HEAP$$ZI$$Base)
#define DMA_HEAP_END_ADDR           (&Image$$DMA_HEAP$$ZI$$Limit)

extern uint32_t                     Image$$SRAMHEAP$$ZI$$Base;
extern uint32_t                     Image$$SRAMHEAP$$ZI$$Limit;
#define SRAM_HEAP_START_ADDR        (&Image$$SRAMHEAP$$ZI$$Base)
#define SRAM_HEAP_END_ADDR          (&Image$$SRAMHEAP$$ZI$$Limit)

extern uint32_t                     __StackTop;
#define STACK_TOP                   (&__StackTop);

extern uint32_t                     Image$$SYS_STACK$$ZI$$Base;
extern uint32_t                     Image$$SYS_STACK$$ZI$$Limit;
#define SYS_STACK_TOP               (&Image$$SYS_STACK$$ZI$$Base)
#define SYS_STACK_BOTTOM            (&Image$$SYS_STACK$$ZI$$Limit)

extern uint32_t                     Image$$FIQ_STACK$$ZI$$Base;
extern uint32_t                     Image$$FIQ_STACK$$ZI$$Limit;
#define FIQ_STACK_TOP               (&Image$$FIQ_STACK$$ZI$$Base)
#define FIQ_STACK_BOTTOM            (&Image$$FIQ_STACK$$ZI$$Limit)

extern uint32_t                     Image$$IRQ_STACK$$ZI$$Base;
extern uint32_t                     Image$$IRQ_STACK$$ZI$$Limit;
#define IRQ_STACK_TOP               (&Image$$IRQ_STACK$$ZI$$Base)
#define IRQ_STACK_BOTTOM            (&Image$$IRQ_STACK$$ZI$$Limit)

extern uint32_t                     Image$$SVC_STACK$$ZI$$Base;
extern uint32_t                     Image$$SVC_STACK$$ZI$$Limit;
#define SVC_STACK_TOP               (&Image$$SVC_STACK$$ZI$$Base)
#define SVC_STACK_BOTTOM            (&Image$$SVC_STACK$$ZI$$Limit)

extern uint32_t                     Image$$ABT_STACK$$ZI$$Base;
extern uint32_t                     Image$$ABT_STACK$$ZI$$Limit;
#define ABT_STACK_TOP               (&Image$$ABT_STACK$$ZI$$Base)
#define ABT_STACK_BOTTOM            (&Image$$ABT_STACK$$ZI$$Limit)

extern uint32_t                     Image$$UND_STACK$$ZI$$Base;
extern uint32_t                     Image$$UND_STACK$$ZI$$Limit;
#define UND_STACK_TOP               (&Image$$UND_STACK$$ZI$$Base)
#define UND_STACK_BOTTOM            (&Image$$UND_STACK$$ZI$$Limit)

static void memmap_help(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "Usage: memmap");
}

static void _memmap(void)
{
    Shell *shell = shellGetCurrent();
    shellPrint(shell, "---------------- STACK ----------------\n");
    
    shellPrint(shell, "SYS  stack: [0x%08x ~ 0x%08x] -- %-4d  B\n",
            SYS_STACK_TOP, SYS_STACK_BOTTOM, ((size_t)SYS_STACK_BOTTOM-(size_t)SYS_STACK_TOP));
    
    shellPrint(shell, "FIQ  stack: [0x%08x ~ 0x%08x] -- %-4d  B\n",
            FIQ_STACK_TOP, FIQ_STACK_BOTTOM, ((size_t)FIQ_STACK_BOTTOM-(size_t)FIQ_STACK_TOP));
    
    shellPrint(shell, "IRQ  stack: [0x%08x ~ 0x%08x] -- %-4d  B\n",
            IRQ_STACK_TOP, IRQ_STACK_BOTTOM, ((size_t)IRQ_STACK_BOTTOM-(size_t)IRQ_STACK_TOP));
    
    shellPrint(shell, "SVC  stack: [0x%08x ~ 0x%08x] -- %-4d  B\n",
            SVC_STACK_TOP, SVC_STACK_BOTTOM, ((size_t)SVC_STACK_BOTTOM-(size_t)SVC_STACK_TOP));
    
    shellPrint(shell, "ABT  stack: [0x%08x ~ 0x%08x] -- %-4d  B\n",
            ABT_STACK_TOP, ABT_STACK_BOTTOM, ((size_t)ABT_STACK_BOTTOM-(size_t)ABT_STACK_TOP));
    
    shellPrint(shell, "UND  stack: [0x%08x ~ 0x%08x] -- %-4d  B\n",
            UND_STACK_TOP, UND_STACK_BOTTOM, ((size_t)UND_STACK_BOTTOM-(size_t)UND_STACK_TOP));
    
    shellPrint(shell, "---------------- HEAP -----------------\n");
    
    shellPrint(shell, "SRAM  heap: [0x%08x ~ 0x%08x] -- %-4d KB\n",
            SRAM_HEAP_START_ADDR, SRAM_HEAP_END_ADDR, ((size_t)SRAM_HEAP_END_ADDR - (size_t)SRAM_HEAP_START_ADDR)/1024UL);
    
    shellPrint(shell, "SMALL heap: [0x%08x ~ 0x%08x] -- %-4d KB\n",
            SMALL_HEAP_START_ADDR, SMALL_HEAP_END_ADDR, ((size_t)SMALL_HEAP_END_ADDR - (size_t)SMALL_HEAP_START_ADDR)/1024UL);

    shellPrint(shell, "DDR   heap: [0x%08x ~ 0x%08x] -- %-4d KB\n",
            HEAP_START_ADDR, HEAP_END_ADDR, ((size_t)HEAP_END_ADDR - (size_t)HEAP_START_ADDR)/1024UL);
    
    shellPrint(shell, "DMA   heap: [0x%08x ~ 0x%08x] -- %-4d KB\n",
            DMA_HEAP_START_ADDR, DMA_HEAP_END_ADDR, ((size_t)DMA_HEAP_END_ADDR - (size_t)DMA_HEAP_START_ADDR)/1024UL);
}


void memmap(int argc, char *argv[])
{
    if (argc != 1)  {
        memmap_help();
        return;
    }

    (void)argv;

    _memmap();
}


/* _attr, _name, _func, _desc */
SHELL_EXPORT_CMD(
SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN,
memmap, memmap, dump memory map);

