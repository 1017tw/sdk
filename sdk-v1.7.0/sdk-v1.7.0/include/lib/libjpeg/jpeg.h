#ifndef _LIBJPEG_WRAPPER_H_
#define _LIBJPEG_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "frame_mgr.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief decode jpeg image from buffer
 * @param[in]   buffer   buffer contain jpeg image
 * @param[in]   buf_len  length of buffer
 * @param[in]   output_format target output format, only FRAME_FMT_RGB888 and FRAME_FMT_RAW8 are supported
 * @return decoded frame with fmt == output_format or NULL if decode fail
 */
frame_buf_t *libjpeg_decode_buffer(uint8_t *buffer, uint32_t buf_len, frame_fmt_t output_format);

/**
 * @brief decode jpeg frame
 * @param[in]   frame   jpeg frame to be decoded
 * @param[in]   output_format target output format, only FRAME_FMT_RGB888 and FRAME_FMT_RAW8 are supported
 * @return decoded frame with fmt == output_format or NULL if decode fail
 */
frame_buf_t *libjpeg_decode(frame_buf_t *frame, frame_fmt_t output_format);

/**
 * @brief encode jpeg, only support encode raw8/yuv422p/i420 frame
 * @param[in]   frame   frame to be encoded, function will not change reference count of this buffer
 * @param[in]   level   factor should be 0 (terrible) to 100 (very good)
 * @return encoded frame
 */
frame_buf_t *libjpeg_encode(frame_buf_t *frame, int level);

/**
 * @brief get width and height info from jpeg buffer
 * @param[in]   buffer   buffer contain jpeg image
 * @param[in]   buf_len  length of buffer
 * @param[in,out]   width  width of jpeg image
 * @param[in,out]   height  height of jpeg image
 * @return 0 if success, others if fail
 */
int libjpeg_get_width_height(uint8_t *buffer, uint32_t buf_len, int *width, int *height);

#ifdef __cplusplus
}
#endif

#endif
