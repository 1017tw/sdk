#ifndef __AIVA_ATTR_H__
#define __AIVA_ATTR_H__

#include "cmsis_gcc.h"
#include "system_AI3100.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CACHE_LINE_SIZE__
#define __CACHE_LINE_SIZE__     (32)
#endif

// Forces code into SRAM instead of flash
// NOTE: when executing on ROM, use `XIP_CONCURR_ATTR` to relocate code into sram region
#   define XIP_CONCURR_ATTR _SECTION_ATTR_IMPL(".sram", __COUNTER__)

#ifdef SRAM_RELOCATE_CODE
#   define SRAM_CODE_ATTR _SECTION_ATTR_IMPL(".sram", __COUNTER__)
#else
#   define SRAM_CODE_ATTR
#endif

// Forces data into DRAM instead of flash
#define DRAM_CODE_ATTR _SECTION_ATTR_IMPL(".dram", __COUNTER__)

// Forces data into SRAM instead of DRAM
#ifdef SRAM_RELOCATE_CODE
#   define SRAM_DATA_ATTR __attribute__((section(".sram1.data")))
#else
#   define SRAM_DATA_ATTR
#endif

// Forces bss into IRAM instead of DRAM
#ifdef SRAM_RELOCATE_CODE
#   define SRAM_BSS_ATTR __attribute__((section(".sram1.bss")))
#else
#   define SRAM_BSS_ATTR
#endif

// Forces data into SRAM instead of DRAM
#define DRAM_DATA_ATTR __attribute__((section(".dram1.data")))

// Forces bss into IRAM instead of DRAM
#define DRAM_BSS_ATTR __attribute__((section(".dram1.bss")))

// Forces data to be 4 bytes aligned
#define WORD_ALIGNED_ATTR __attribute__((aligned(4)))

// Forces data to be cache line aligned
#define CACAHE_ALINGED_ATTR __attribute__((aligned(__CACHE_LINE_SIZE__))

// Forces a function to be inlined
#define FORCE_INLINE_ATTR static inline __attribute__((always_inline))

// Forces a string into DRAM instead of flash
// Use as printf(DRAM_STR("Hello world!\n"));
#define DRAM_STR(str) (__extension__({static const DRAM_ATTR char __c[] = (str); (const char *)&__c;}))

// Forces to not inline function
#define NOINLINE_ATTR __attribute__((noinline))

// Force function to be weak
#define WEAK_ATTR __attribute__((weak))

// Implementation for a unique custom section
//
// This prevents gcc producing "x causes a section type conflict with y"
// errors if two variables in the same source file have different linkage (maybe const & non-const) but are placed in the same custom section
//
// Using unique sections also means --gc-sections can remove unused
// data with a custom section type set
#define _SECTION_ATTR_IMPL(SECTION, COUNTER) __attribute__((section(SECTION "." _COUNTER_STRINGIFY(COUNTER))))

#define _COUNTER_STRINGIFY(COUNTER) #COUNTER

#   define XIP_CONCURR_CALL(__func_ptr) \
    do { \
    if (is_xip_exec_mode()) { \
        asm volatile ( "CPSID i" ::: "memory" ); \
        asm volatile ("dsb"); \
        asm volatile ("isb"); \
        __func_ptr;   \
        asm volatile ( "CPSIE i" ::: "memory" ); \
        asm volatile ("dsb"); \
        asm("isb"); \
    } else {	\
        __func_ptr;   \
    }	\
    } while (0)
#   define XIP_CONCURR_CALL_RET(__r, __func_ptr) \
    do { \
    if (is_xip_exec_mode()) { \
        asm volatile ( "CPSID i" ::: "memory" ); \
        asm volatile ("dsb"); \
        asm volatile ("isb"); \
        __r = __func_ptr; \
        asm volatile ( "CPSIE i" ::: "memory" ); \
        asm volatile ("dsb"); \
        asm volatile ("isb"); \
    } else {	\
        __func_ptr;   \
    }	\
    } while (0)

#ifdef __cplusplus
}
#endif
#endif /* __AIVA_ATTR_H__ */
