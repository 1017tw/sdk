#include "mem_ld.h" 

MEMORY
{
  SRAM_CODE (WX)  : ORIGIN = __SRAM_SEG_BASE,    LENGTH = __SRAM_CODE_SIZE
  SRAM_HEAP (WX)  : ORIGIN = __SRAM_HEAP_BASE,   LENGTH = __SRAM_HEAP_SIZE
  SRAM_STACK (WX) : ORIGIN = __SRAM_STACK_BASE,  LENGTH = __STACK_SIZE_ALL
#ifdef XIP_EXEC_DEBUG
  ROM (RX)      : ORIGIN = 0x804100A0,       LENGTH = 0x00400000
  DDR_HEAP (WX) : ORIGIN = __DDR1_BASE,      LENGTH = __DDR1_SIZE - __DMA_MEM_SIZE
#else
  /*ROM (RX)    : ORIGIN = 0x300100A0,       LENGTH = 0x00800000*/
  ROM (RX)      : ORIGIN = CONFIG_XIP_BOOT_ADDR+0xA0, LENGTH = 0x00800000
  DDR_HEAP (WX) : ORIGIN = __DDR_HEAP_BASE,       LENGTH = __DDR_HEAP_SIZE
#endif
  DMA (WX) : ORIGIN = __DMA_MEM_BASE,   LENGTH = __DMA_MEM_SIZE
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        Image$$VECTORS$$Base = .;
        * (RESET)
        KEEP(*(.isr_vector))
        Image$$VECTORS$$Limit = .;

        *(SVC_TABLE)
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        . = ALIGN(4);

        Image$$RO_DATA$$Base = .;
        *(.rodata*)
        Image$$RO_DATA$$Limit = .;
        
        . = ALIGN(4);
        /* cons cmd struct data */
        PROVIDE (__cons_cmd_start = .);
        KEEP(*(.cons_cmd*))
        PROVIDE (__cons_cmd_end = .);

        . = ALIGN(4);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;

        . = ALIGN(4);
        _device_list_start = .;
        KEEP (*(.udevice_list))
        _device_list_end = .;

        . = ALIGN(4);
        _MODULE_TEST_start = .;
        KEEP (*(MODULE_TEST))
        _MODULE_TEST_end = .;
        
        
        KEEP(*(.eh_frame*))
    } > ROM

    .bootSectionBlock  :
    {
        __BOOT_SECTION_START = .;
        KEEP(*(.bootSection))
        __BOOT_SECTION_END = .;
    } > ROM

    .ARM.extab : 
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ROM

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > ROM
    __exidx_end = .;


    /*
    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (__etext)
        LONG (__data_start__)
        LONG (__data_end__ - __data_start__)
        __copy_table_end__ = .;
    } > DDR
    */

    /*.zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (__bss_start__)
        LONG (__bss_end__ - __bss_start__)
        __zero_table_end__ = .;
    } > DDR*/

    . = ALIGN(4);
    __copy_head = .;
    __etext = .;

    .data : AT(__etext)
    {
        Image$$RW_DATA$$Base = .;
        __copy_start__ = .;
        __data_start__ = .;
        *(vtable)
        *(.data*)
        Image$$RW_DATA$$Limit = .;

        . = ALIGN(4);
        /* preinit data */
        PROVIDE (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE (__fini_array_end = .);

        . = ALIGN(4);
        _segger_rtt_start = .;
        KEEP (*(.segger_rtt))
        _segger_rtt_end = .;
        
        . = ALIGN(32);
        __sram_text_data_start__ = .;
        KEEP(*(.sram .sram.*))   /* catch stray SRAM_CODE_ATTR */
        . += 32;
        . = ALIGN(32);
        KEEP(*(.sram1 .sram1.*)) /* catch stray SRAM_DATA_ATTR */
        __sram_text_data_end__ = .;

        . = ALIGN(32);
        /* All data end */
        __copy_end__ = .;
        __data_end__ = .;

    } > SRAM_CODE

    .bss (NOLOAD):
    {
        Image$$ZI_DATA$$Base = .;
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
        Image$$ZI_DATA$$Limit = .;
        __end__ = .;
        end = __end__;
    } > SRAM_CODE
    
    .small_heap (NOLOAD):
    {
        . = ALIGN(8);
        Image$$SMALL_HEAP$$ZI$$Base = .;
        . += __SMALL_MEM_POOL_SIZE;
        Image$$SMALL_HEAP$$ZI$$Limit = .;
    } > DDR_HEAP

    .heap (NOLOAD):
    {
        . = ALIGN(8);
        Image$$HEAP$$ZI$$Base = .;
        . += __DDR_HEAP_SIZE - __SMALL_MEM_POOL_SIZE;
        Image$$HEAP$$ZI$$Limit = .;
        __HeapLimit = .;
    } > DDR_HEAP 

    .dma (NOLOAD):
    {
        . = ALIGN(64);
        *(.dma*)
        . = ALIGN(64);
        Image$$DMA_HEAP$$ZI$$Base  = .;
        . += __DMA_HEAP_SIZE;
        Image$$DMA_HEAP$$ZI$$Limit = .;
    } > DMA
    
    .sramheap (NOLOAD):
    {
		/*. = ORIGIN(SRAM) + __SRAM_CODE_SIZE;*/
        . = ALIGN(8);
        Image$$SRAMHEAP$$ZI$$Base = .;
        . += __SRAM_HEAP_SIZE;
        Image$$SRAMHEAP$$ZI$$Limit = .;
        __SramHeapLimit = .;
    } > SRAM_HEAP

    .stack (NOLOAD):
    {
        /*. = ORIGIN(SRAM) + __SRAM_STACK_OFFSET;*/
        . = ALIGN(8);
        __StackTop = .;
        Image$$SYS_STACK$$ZI$$Base = .;
        . += __SYS_STACK_SIZE;
        Image$$SYS_STACK$$ZI$$Limit = .;
        __stack = .;

        Image$$FIQ_STACK$$ZI$$Base = .;
        . += __FIQ_STACK_SIZE;
        Image$$FIQ_STACK$$ZI$$Limit = .;

        Image$$IRQ_STACK$$ZI$$Base = .;
        . += __IRQ_STACK_SIZE;
        Image$$IRQ_STACK$$ZI$$Limit = .;

        Image$$SVC_STACK$$ZI$$Base = .;
        . += __SVC_STACK_SIZE;
        Image$$SVC_STACK$$ZI$$Limit = .;

        Image$$ABT_STACK$$ZI$$Base = .;
        . += __ABT_STACK_SIZE;
        Image$$ABT_STACK$$ZI$$Limit = .;

        Image$$UND_STACK$$ZI$$Base = .;
        . += __UND_STACK_SIZE;
        Image$$UND_STACK$$ZI$$Limit = .;
        
    } > SRAM_STACK
    
}
