#ifndef _AIVA_SLEEP_H
#define _AIVA_SLEEP_H

#include <stdint.h>
#include "syslog.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief  aiva_usleep, cpu sleep in us, context switch will be triggered in RTOS
 * 
 * @param[in] usec      us to sleep
 * 
 * @return int, default as 0  
 */
int aiva_usleep(uint64_t usec);

/** 
 * @brief  aiva_msleep, cpu sleep in ms, context switch will be triggered in RTOS
 * 
 * @param[in] msec      ms to sleep
 * 
 * @return int, default as 0  
 */
int aiva_msleep(uint32_t msec);

/** 
 * @brief  aiva_sleep, cpu sleep in seconds, context switch will be triggered in RTOS
 * 
 * @param[in] seconds   seconds to sleep
 * 
 * @return int, default as 0  
 */
int aiva_sleep(uint32_t seconds);

/** 
 * @brief  aiva_busy_delay_us, busy delay, no context switch
 * 
 * @param[in] usec      us to delay
 * 
 * @return int, default as 0  
 */
int aiva_busy_delay_us(uint64_t usec);

/** 
 * @brief  aiva_busy_delay_ms, busy delay, no context switch
 * 
 * @param[in] msec      ms to delay
 * 
 * @return int, default as 0  
 */
int aiva_busy_delay_ms(uint32_t msec);

/** 
 * @brief  aiva_enter_sleep_mode, enter sleep mode with slower clk to save power
 * 
 * @return int, default as 0  
 */
int aiva_enter_sleep_mode(void);

/** 
 * @brief  aiva_exit_sleep_mode, exit sleep mode
 * 
 * @return int, default as 0 
 */
int aiva_exit_sleep_mode(void);

/** 
 * @brief  aiva_get_curr_ms, get current elapsed time from boot in ms
 * 
 * @return int, elapsed time in ms 
 */
uint32_t aiva_get_curr_ms(void);

/** 
 * @brief  is_in_aiva_sleep_mode 
 * 
 * @return int, 0 - non-sleeping, 1 - sleeping
 */
int is_in_aiva_sleep_mode(void);

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */

#define typecheck(type, x)          \
({  type __dummy;                   \
    typeof(x) __dummy2;             \
    (void)(&__dummy == &__dummy2);  \
    1;                              \
 })

#define time_after(a, b)            \
    (typecheck(unsigned long, a) && \
     typecheck(unsigned long, b) && \
     ((long)((b) - (a)) < 0))

#define time_before(a, b) time_after(b, a)

#define PRINT_CURR_LINE_MS() LOGW(__func__, "line(%d): %d ms", __LINE__, aiva_get_curr_ms())

#ifdef __cplusplus
}
#endif

#endif /* _BSP_SLEEP_H */

