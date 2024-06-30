#ifndef _DRIVER_IO_H
#define _DRIVER_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define readb(addr) (*(volatile uint8_t  *)(uintptr_t)(addr))
#define readw(addr) (*(volatile uint16_t *)(uintptr_t)(addr))
#define readl(addr) (*(volatile uint32_t *)(uintptr_t)(addr))
#define readq(addr) (*(volatile uint64_t *)(uintptr_t)(addr))

#define writeb(v, addr)                                                     \
    do {                                                                    \
        (*(volatile uint8_t *)(uintptr_t)(addr)) = (v);                     \
    } while (0)

#define writew(v, addr)                                                     \
    do {                                                                    \
        (*(volatile uint16_t *)(uintptr_t)(addr)) = (v);                    \
    } while (0)

#define writel(v, addr)                                                     \
    do {                                                                    \
        (*(volatile uint32_t *)(uintptr_t)(addr)) = (v);                    \
    } while (0)

#define writeq(v, addr)                                                     \
    do {                                                                    \
        (*(volatile uint64_t *)(uintptr_t)(addr)) = (v);                    \
    } while (0)

#define MEM_READ8                           readb
#define MEM_READ16                          readw
#define MEM_READ32                          readl
#define MEM_READ64                          readq

#define MEM_WRITE8(addr, data)              writeb(data, addr)
#define MEM_WRITE16(addr, data)             writew(data, addr)
#define MEM_WRITE32(addr, data)             writel(data, addr)
#define MEM_WRITE64(addr, data)             writeq(data, addr)

#define MEM_READ32_WITH_MASK(addr, mask)    (MEM_READ32(addr) & (mask))

#define MEM_WRITE32_WITH_MASK(addr, data, mask)                 \
    do {                                                        \
        uint32_t val = MEM_READ32(addr);                        \
        val = ((data) & (mask)) | ((val) & (~(mask)));          \
        MEM_WRITE32(addr, val);                                 \
    } while (0)


typedef uint32_t (*readl_func_t)(volatile void *addr);
typedef void     (*writel_func_t)(volatile void *addr, uint32_t data);
typedef int      (*checkl_func_t)(volatile void *addr, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_IO_H */
