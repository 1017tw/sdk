#ifndef _DRIVER_GPIO_H
#define _DRIVER_GPIO_H

#include <stdint.h>
#include <stddef.h>
#include "memory_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------*/
/*  typedef                                                                  */
/* --------------------------------------------------------------------------*/
enum _gpio_drive_mode_t
{
    GPIO_DM_INPUT,
    GPIO_DM_INPUT_PULL_DOWN,
    GPIO_DM_INPUT_PULL_UP,
    GPIO_DM_OUTPUT,
};
typedef int32_t gpio_drive_mode_t;

/* interrupt mode */
enum _gpio_intr_mode_t {
    GPIO_NO_INTR = 0,
    GPIO_INTR_POS_EDGE,
    GPIO_INTR_NEG_EDGE,
    GPIO_INTR_BOTH_EDGE,
    GPIO_INTR_LOW_LEVEL,
    GPIO_INTR_HIGH_LEVEL,
};
typedef int32_t gpio_intr_mode_t;

/* PIN value */
enum _gpio_pin_value_t
{
    GPIO_PV_LOW = 0,
    GPIO_PV_HIGH
};
typedef int32_t gpio_pin_value_t;

/* PIN numbers, from Bank-A to Bank-C */
typedef enum {
    /* BANK A */
    GPIOA_PIN0  = 0,
    GPIOA_PIN1  = 1,
    GPIOA_PIN2  = 2,
    GPIOA_PIN3  = 3,
    GPIOA_PIN4  = 4,
    GPIOA_PIN5  = 5,
    GPIOA_PIN6  = 6,
    GPIOA_PIN7  = 7,
    GPIOA_PIN8  = 8,
    GPIOA_PIN9  = 9,
    GPIOA_PIN10 = 10,
    GPIOA_PIN11 = 11,
    GPIOA_PIN12 = 12,
    GPIOA_PIN13 = 13,
    GPIOA_PIN14 = 14,
    GPIOA_PIN15 = 15,
    GPIOA_PIN16 = 16,
    GPIOA_PIN17 = 17,
    GPIOA_PIN18 = 18,
    GPIOA_PIN19 = 19,
    GPIOA_PIN20 = 20,
    GPIOA_PIN21 = 21,
    GPIOA_PIN22 = 22,
    GPIOA_PIN23 = 23,
    GPIOA_PIN24 = 24,
    GPIOA_PIN25 = 25,
    GPIOA_PIN26 = 26,
    GPIOA_PIN27 = 27,
    GPIOA_PIN28 = 28,
    GPIOA_PIN29 = 29,
    GPIOA_PIN30 = 30,
    GPIOA_PIN31 = 31,
    /* BANK B */
    GPIOB_PIN0  = 32,
    GPIOB_PIN1  = 33,
    GPIOB_PIN2  = 34,
    GPIOB_PIN3  = 35,
    GPIOB_PIN4  = 36,
    GPIOB_PIN5  = 37,
    GPIOB_PIN6  = 38,
    GPIOB_PIN7  = 39,
    GPIOB_PIN8  = 40,
    GPIOB_PIN9  = 41,
    GPIOB_PIN10 = 42,
    GPIOB_PIN11 = 43,
    GPIOB_PIN12 = 44,
    GPIOB_PIN13 = 45,
    GPIOB_PIN14 = 46,
    GPIOB_PIN15 = 47,
    GPIOB_PIN16 = 48,
    GPIOB_PIN17 = 49,
    GPIOB_PIN18 = 50,
    GPIOB_PIN19 = 51,
    GPIOB_PIN20 = 52,
    GPIOB_PIN21 = 53,
    GPIOB_PIN22 = 54,
    GPIOB_PIN23 = 55,
    GPIOB_PIN24 = 56,
    GPIOB_PIN25 = 57,
    GPIOB_PIN26 = 58,
    GPIOB_PIN27 = 59,
    GPIOB_PIN28 = 60,
    GPIOB_PIN29 = 61,
    GPIOB_PIN30 = 62,
    /* BANK C */
    GPIOC_PIN0  = 63,
    GPIOC_PIN1  = 64,
    GPIOC_PIN2  = 65,
    GPIOC_PIN3  = 66,
    GPIOC_PIN4  = 67 ,
    GPIOC_PIN5  = 68,
    GPIOC_PIN6  = 69,
    GPIO_MAX_PINNO,
} gpio_pin_t;

/* To be compatible with previous pin number definition */
/* Define PINs without BANK names */
enum {
    /* BANK A */
    GPIO_PIN0  = 0,
    GPIO_PIN1  = 1,
    GPIO_PIN2  = 2,
    GPIO_PIN3  = 3,
    GPIO_PIN4  = 4,
    GPIO_PIN5  = 5,
    GPIO_PIN6  = 6,
    GPIO_PIN7  = 7,
    GPIO_PIN8  = 8,
    GPIO_PIN9  = 9,
    GPIO_PIN10 = 10,
    GPIO_PIN11 = 11,
    GPIO_PIN12 = 12,
    GPIO_PIN13 = 13,
    GPIO_PIN14 = 14,
    GPIO_PIN15 = 15,
    GPIO_PIN16 = 16,
    GPIO_PIN17 = 17,
    GPIO_PIN18 = 18,
    GPIO_PIN19 = 19,
    GPIO_PIN20 = 20,
    GPIO_PIN21 = 21,
    GPIO_PIN22 = 22,
    GPIO_PIN23 = 23,
    GPIO_PIN24 = 24,
    GPIO_PIN25 = 25,
    GPIO_PIN26 = 26,
    GPIO_PIN27 = 27,
    GPIO_PIN28 = 28,
    GPIO_PIN29 = 29,
    GPIO_PIN30 = 30,
    GPIO_PIN31 = 31,
    /* BANK B */
    GPIO_PIN32 = 32,
    GPIO_PIN33 = 33,
    GPIO_PIN34 = 34,
    GPIO_PIN35 = 35,
    GPIO_PIN36 = 36,
    GPIO_PIN37 = 37,
    GPIO_PIN38 = 38,
    GPIO_PIN39 = 39,
    GPIO_PIN40 = 40,
    GPIO_PIN41 = 41,
    GPIO_PIN42 = 42,
    GPIO_PIN43 = 43,
    GPIO_PIN44 = 44,
    GPIO_PIN45 = 45,
    GPIO_PIN46 = 46,
    GPIO_PIN47 = 47,
    GPIO_PIN48 = 48,
    GPIO_PIN49 = 49,
    GPIO_PIN50 = 50,
    GPIO_PIN51 = 51,
    GPIO_PIN52 = 52,
    GPIO_PIN53 = 53,
    GPIO_PIN54 = 54,
    GPIO_PIN55 = 55,
    GPIO_PIN56 = 56,
    GPIO_PIN57 = 57,
    GPIO_PIN58 = 58,
    GPIO_PIN59 = 59,
    GPIO_PIN60 = 60,
    GPIO_PIN61 = 61,
    GPIO_PIN62 = 62,
    /* BANK C */
    GPIO_PIN63 = 63,
    GPIO_PIN64 = 64,
    GPIO_PIN65 = 65,
    GPIO_PIN66 = 66,
    GPIO_PIN67 = 67 ,
    GPIO_PIN68 = 68,
    GPIO_PIN69 = 69,
};

/* Bank number */
typedef enum {
    GPIO_BANK_A = 0,
    GPIO_BANK_B,
    GPIO_BANK_C,
    GPIO_MAX_BANK,
} gpio_bank_t;

/**
 * @brief       Initialize GPIO, enable clk and reset hw
 * 
 * @param[in]   b               GPIO bank number
 *
 * @return      Result
 *     - 0      Success
 *     - Other Fail
 */
int gpio_init(gpio_bank_t b);

/**
 * @brief       Set GPIO drive mode
 *
 * @param[in]   pin             GPIO pin
 * @param[in]   mode            GPIO pin drive mode
 */
void gpio_set_drive_mode(gpio_pin_t pin, gpio_drive_mode_t mode);

/**
 * @brief       Get GPIO pin value
 *
 * @param[in]   pin             GPIO pin
 * @return      Pin value
 *
 *     - GPIO_PV_Low     GPIO pin low
 *     - GPIO_PV_High    GPIO pin high
 */
gpio_pin_value_t gpio_get_pin(gpio_pin_t pin);

/**
 * @brief       Set GPIO pin value
 *
 * @param[in]   pin             GPIO pin
 * @param[in]   value           GPIO pin value
 */
void gpio_set_pin(gpio_pin_t pin, gpio_pin_value_t value);

typedef int (*gpio_irq_cb_t)(void *ctx);

/** 
 * @brief  gpio_irq_register    register gpio irq function
 * 
 * @param gpio_pin              gpio pin
 * @param mode                  interrupt mode
 * @param cb                    callback function pointer
 * @param ctx                   callback context
 * 
 * @return   
 */
int gpio_irq_register(uint8_t gpio_pin, gpio_intr_mode_t mode, 
        gpio_irq_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_GPIO_H */

