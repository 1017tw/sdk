#ifndef __SPI_CONCURR_XIP_H__
#define __SPI_CONCURR_XIP_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "aiva_attr.h"

#ifdef __cplusplus
extern "C" {
#endif

int  XIP_CONCURR_ATTR spi_xip_set_wait_cycles(uint8_t wait_cycles);

int  XIP_CONCURR_ATTR spi_xip_set_baudr(int baudr);

void XIP_CONCURR_ATTR spi_xip_reset_spi0(void);

void XIP_CONCURR_ATTR spi_xip_enable(void);

void XIP_CONCURR_ATTR spi_xip_disable(void);

int  XIP_CONCURR_ATTR spi_xip_send_std_mode(const uint8_t *cmd_buff, int cmd_len, const uint8_t *tx_buff, int tx_len);

int  XIP_CONCURR_ATTR spi_xip_recv_std_mode(const uint8_t *cmd_buff, int cmd_len, uint8_t *rx_buff, int rx_len);

int  XIP_CONCURR_ATTR spi_xip_send_quad_mode(const uint32_t *cmd_buff, int cmd_len, const uint8_t *tx_buff, int tx_len);

int  XIP_CONCURR_ATTR spi_xip_recv_quad_mode( const uint32_t *cmd_buff, int cmd_len, uint8_t *rx_buff, int rx_len);

#ifdef __cplusplus
}
#endif

#endif // __SPI_CONCURR_XIP_H__
