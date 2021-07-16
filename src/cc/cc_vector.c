#include <arm_neon.h>
#include <stdio.h>

#include "cc.h"

#define VECTOR_WIDTH 8

static inline uint8_t avg2(uint8_t e1, uint8_t e2) {
    return (e1 + e2) >> 1;
}

static inline uint8_t avg4(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) {
    return (e1 + e2 + e3 + e4) >> 2;
}

void cc_vector(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    uint8x8x3_t buff;
    int16x8x3_t rgb;
    int16x8x3_t ycc;
    int16x8_t temp;

    int16x4_t coeff1;
    int16x4_t coeff2;

    coeff1[0] = 66;
    coeff1[1] = 128; // 129
    coeff1[2] = 25;

    coeff1[3] = -38;
    coeff2[0] = -74;
    coeff2[1] = 112;

    coeff2[2] = -94;
    coeff2[3] = -18;

    int16x8_t val_16 = vdupq_n_s16(16);
    int16x8_t val_128 = vdupq_n_s16(128);

    int16x8_t val_y_max = vdupq_n_s16(Y_MAX_VAL);
    int16x8_t val_c_max = vdupq_n_s16(C_MAX_VAL);

    for (uint32_t pixel = 0; pixel < rgb_width*rgb_height; pixel += 8) {
        buff = vld3_u8(rgb_data+pixel*3);

        rgb.val[0] = vreinterpretq_s16_u16(vmovl_u8(buff.val[0]));
        rgb.val[1] = vreinterpretq_s16_u16(vmovl_u8(buff.val[1]));
        rgb.val[2] = vreinterpretq_s16_u16(vmovl_u8(buff.val[2]));

        ycc.val[0] = vmulq_lane_s16(rgb.val[0], coeff1, 0);
        ycc.val[0] = vshrq_n_s16(ycc.val[0], 8);
        temp = vmulq_lane_s16(rgb.val[1], coeff1, 1);
        temp = vshrq_n_s16(temp, 8);
        ycc.val[0] = vaddq_s16(ycc.val[0], temp);
        temp = vmulq_lane_s16(rgb.val[2], coeff1, 2);
        temp = vshrq_n_s16(temp, 8);
        ycc.val[0] = vaddq_s16(ycc.val[0], temp);
        ycc.val[0] = vaddq_s16(ycc.val[0], val_16);
        ycc.val[0] = vminq_s16(ycc.val[0], val_y_max);
        ycc.val[0] = vmaxq_s16(ycc.val[0], val_16);

        ycc.val[1] = vmulq_lane_s16(rgb.val[0], coeff1, 3);
        ycc.val[1] = vshrq_n_s16(ycc.val[1], 8);
        temp = vmulq_lane_s16(rgb.val[1], coeff2, 0);
        temp = vshrq_n_s16(temp, 8);
        ycc.val[1] = vaddq_s16(ycc.val[1], temp);
        temp = vmulq_lane_s16(rgb.val[2], coeff2, 1);
        temp = vshrq_n_s16(temp, 8);
        ycc.val[1] = vaddq_s16(ycc.val[1], temp);
        ycc.val[1] = vaddq_s16(ycc.val[1], val_128);
        ycc.val[1] = vminq_s16(ycc.val[1], val_c_max);
        ycc.val[1] = vmaxq_s16(ycc.val[1], val_16);

        ycc.val[2] = vmulq_lane_s16(rgb.val[0], coeff2, 1);
        ycc.val[2] = vshrq_n_s16(ycc.val[2], 8);
        temp = vmulq_lane_s16(rgb.val[1], coeff2, 2);
        temp = vshrq_n_s16(temp, 8);
        ycc.val[2] = vaddq_s16(ycc.val[2], temp);
        temp = vmulq_lane_s16(rgb.val[2], coeff2, 3);
        temp = vshrq_n_s16(temp, 8);
        ycc.val[2] = vaddq_s16(ycc.val[2], temp);
        ycc.val[2] = vaddq_s16(ycc.val[2], val_128);
        ycc.val[2] = vminq_s16(ycc.val[2], val_c_max);
        ycc.val[2] = vmaxq_s16(ycc.val[2], val_16);

        buff.val[0] = vqmovun_s16(ycc.val[0]);
        buff.val[1] = vqmovun_s16(ycc.val[1]);
        buff.val[2] = vqmovun_s16(ycc.val[2]);

        vst3_u8(rgb_data+pixel*3, buff);
    }

    // Convert pixel values from RGB to YCC
    for (uint32_t row = 0; row < rgb_height; ++row) {
        for (uint32_t col = 0; col < rgb_width; ++col) {
            const uint32_t ycc_y_idx = (col + (row * rgb_width));
            const uint32_t idx = 3 * ycc_y_idx;

            ycc_data[ycc_y_idx] = rgb_data[idx];
            uint8_t* g = rgb_data+(idx+1);
            uint8_t* b = rgb_data+(idx+2);  

            // Process 4x4 clusters for downsampling
            if ((row % 2 == 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;
                const uint32_t up_idx = idx - (rgb_width*3);
                const uint32_t up_left_idx = (idx - 3) - (rgb_width*3);

                *g = avg4(*g, rgb_data[left_idx+1], rgb_data[up_idx+1], rgb_data[up_left_idx+1]);
                *b = avg4(*b, rgb_data[left_idx+2], rgb_data[up_idx+2], rgb_data[up_left_idx+2]);

                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
            }
            // Process 1x2 terminal row clusters
            else if ((row == rgb_height - 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;

                *g = avg2(*g, rgb_data[left_idx+1]);
                *b = avg2(*b, rgb_data[left_idx+2]);

                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
            }
            // Process 2x1 terminal column clusters
            else if ((col == rgb_width - 1) && (row % 2 == 1)) {
                const uint32_t up_idx = idx - (rgb_width*3);

                *g = avg2(*g, rgb_data[up_idx+1]);
                *b = avg2(*b, rgb_data[up_idx+2]);

                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
           }
        }
    }
}
