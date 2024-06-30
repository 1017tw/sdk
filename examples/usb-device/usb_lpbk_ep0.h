#ifndef __USB_LPBK_EP0_H__
#define __USB_LPBK_EP0_H__

#include "no_os_defs.h"
#include "usb_lpbk.h"

#ifdef __cplusplus
extern "C" {
#endif

int usb_lpbk_get_desc(struct usb_gadget *gadget, const usb_device_request_t *ctrl);

int usb_lpbk_enable_eps(struct usb_gadget *gadget, f_loopback_t *lpbk);

int usb_lpbk_disable_eps(struct usb_gadget *gadget, f_loopback_t *lpbk);

#ifdef __cplusplus
}    
#endif

#endif // __USB_LPBK_EP0_H__
