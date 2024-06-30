#ifndef __MJPEG_H__
#define __MJPEG_H__

#include "frame_mgr.h"

typedef enum _mjpeg_comp_level_t
{
    MJPEG_COMP_LEVEL_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
    MJPEG_COMP_LEVEL_HIGH          = 1,    /**< high compressing rate, %75 quality */
    MJPEG_COMP_LEVEL_LOW           = 2,    /**< low compressing rate */
    MJPEG_COMP_LEVEL_01_PERCENT    = 3,    /**< %01 quality */
    MJPEG_COMP_LEVEL_10_PERCENT    = 4,    /**< %10 quality */
    MJPEG_COMP_LEVEL_20_PERCENT    = 5,    /**< %20 quality */
    MJPEG_COMP_LEVEL_30_PERCENT    = 6,    /**< %30 quality */
    MJPEG_COMP_LEVEL_40_PERCENT    = 7,    /**< %40 quality */
    MJPEG_COMP_LEVEL_50_PERCENT    = 8,    /**< %50 quality */
    MJPEG_COMP_LEVEL_60_PERCENT    = 9,    /**< %60 quality */
    MJPEG_COMP_LEVEL_70_PERCENT    = 10,   /**< %70 quality */
    MJPEG_COMP_LEVEL_80_PERCENT    = 11,   /**< %80 quality */
    MJPEG_COMP_LEVEL_99_PERCENT    = 12,   /**< %99 quality */
    MJPEG_COMP_LEVEL_MAX                   /**< upper border (only for an internal evaluation) */
} mjpeg_comp_level_t;

#ifdef __cplusplus
extern "C" {
#endif
/* --------------------------------------------------------------------------*/
/** 
 * @brief  mjpeg_compress, compress one frame with jpeg
 * 
 * @param frame         input frame
 * @param level         compressing level
 * 
 * @return   NULL: error
 *
 */
/* --------------------------------------------------------------------------*/
frame_buf_t *mjpeg_compress(frame_buf_t *frame, mjpeg_comp_level_t level);

#ifdef __cplusplus
}
#endif

#endif // __MJPEG_H__
