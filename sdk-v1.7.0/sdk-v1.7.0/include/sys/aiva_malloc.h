/**
 * @file aiva_malloc.h
 * @brief A simple fast malloc implementation for embedded system
 *
 * aiva malloc include two different malloc block, default aiva allocator
 * and tlsf allocator.
 *
 * default aiva allocator is designed to handle big memory request and has more
 * header overhead when malloc memory; tlsf allocator is used to handle small
 * memory request, tlsf has small overhead when malloc memory.
 * 
 * we use following memory allocate strategy to avoid memory fragment
 * 
 * if (request_memroy_size <= SMALL_MEM_SIZE)
 * {
 *      use tlsf allocator
 * }
 * else
 * {
 *      use default aiva allocator
 * }
 *
 * call aiva_default_print_memory_info() and aiva_tlsf_print_memory_info() to
 * print current memory info of differnet allocator
 * 
 * call aiva_default_get_max_free_heap_size() and aiva_tlsf_get_max_free_heap_size()
 * to print current maximum free heap block size
 */

#ifndef __AIVA_MALLOC_H__
#define __AIVA_MALLOC_H__

#include <stdint.h>
#include <stddef.h>

// #define SMALL_MEM_SIZE                  (1060) // make sure not smaller than db item(1060)
// #define SMALL_MEM_POOL_SIZE             (300*1024+100*1060 + 496) = 413696 // *100 for 100 user +496 for 32align
// #define SMALL_MEM_POOL_SIZE             (260*1024+100*1060 + 16) = 372256 // *100 for 100 user +16 for 32align
// #define SMALL_MEM_POOL_SIZE             (250*1024+100*1060 + 16) = 362016 // *100 for 100 user +16 for 32align
//#define SMALL_MEM_SIZE                  (CONFIG_SMALL_MEM_SIZE_BYTE)

#ifdef __cplusplus
extern "C" {
#endif

/* memory pool */
typedef struct _mem_pool_t {
    int      has_inited;
    void    *start_addr;
    void    *last_addr;
    void    *cur_avail_addr;
    void *   pimpl;
    /* debug purpose */
    size_t   num_fragments;
    size_t   stat_total;
} __attribute__((packed, aligned(4)))mem_pool_t;

int check_heap(mem_pool_t *mem_pool);

void *aiva_sram_malloc(size_t size);
void *aiva_sram_calloc(size_t nitems, size_t size);
void *aiva_sram_realloc(void *ptr, size_t size);
void aiva_sram_free(void *firstbyte);

void* aiva_malloc(size_t size);
void* aiva_malloc_aligned(size_t size, const uint32_t align_size);
void  aiva_free(void *userptr);
void* aiva_calloc(size_t nitems, size_t size);
void* aiva_realloc(void *ptr, size_t size);

void* aiva_dma_alloc(size_t size);
void* aiva_dma_alloc_aligned(size_t size, const uint32_t align_size);
void  aiva_dma_free(void *userptr);

extern mem_pool_t g_norm_mem_pool;
extern void aiva_default_print_memory_info(mem_pool_t *mem_pool);
extern void aiva_tlsf_print_memory_info();

#ifdef __cplusplus
}
#endif

#endif
