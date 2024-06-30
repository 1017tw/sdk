#ifndef __LIB_WRAP_AFFINE_H__
#define __LIB_WRAP_AFFINE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "frame_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sim_trans_matrix_t {
    double m[9];
} sim_trans_matrix_t;

typedef struct _sim_point_t {
    double x;
    double y;
} sim_point_t;

int get_sim_transform(sim_trans_matrix_t *sim_trans_matrix, const sim_point_t *src_points, const sim_point_t *dst_points, int points_num, int reflective);

int warpAffine_bgr(const unsigned char *src_data, int src_width, int src_height, 
		unsigned char *dst_data, int dst_width, int dst_height, const sim_trans_matrix_t *M0);

int warpAffine_rgba(const unsigned char *src_data, int src_width, int src_height, 
		unsigned char *dst_data, int dst_width, int dst_height, const sim_trans_matrix_t *M0);

int warpAffine_gray(const unsigned char *src_data, int src_width, int src_stride, int src_height, 
		unsigned char *dst_data, int dst_width, int dst_stride, int dst_height, const sim_trans_matrix_t *M0);

void warpAffine_yuv422sp(const frame_buf_t *src_frame, const sim_trans_matrix_t *M0, frame_buf_t *aligned_frame);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // __LIB_WRAP_AFFINE_H__
