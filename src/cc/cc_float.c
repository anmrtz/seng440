#include <stdint.h>

#include "cc.h"

static inline void clip_f(float* val, float max) {
    if (*val < YCC_MIN_VAL)
        *val = YCC_MIN_VAL;
    else if (*val > max)
        *val = max;
}

static inline uint8_t avg2(uint8_t e1, uint8_t e2) {
    return (e1 + e2) >> 1;
}

static inline uint8_t avg4(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) {
    return (e1 + e2 + e3 + e4) >> 2;
}

void cc_float(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    // Convert pixel values from RGB to YCC
    for (uint32_t row = 0; row < rgb_height; ++row) {
        for (uint32_t col = 0; col < rgb_width; ++col) {
            const uint32_t ycc_y_idx = (col + (row * rgb_width));
            const uint32_t idx = 3 * ycc_y_idx;

            uint8_t* r = rgb_data+(idx);
            uint8_t* g = rgb_data+(idx+1);
            uint8_t* b = rgb_data+(idx+2);                

            // Convert RGB values to YCC
            {
                const float r_val = *r;
                const float g_val = *g;
                const float b_val = *b;

                float y  =  16.0f + 0.257f*r_val + 0.504f*g_val + 0.098f*b_val;
                float cb = 128.0f - 0.148f*r_val - 0.291f*g_val + 0.439f*b_val;
                float cr = 128.0f + 0.439f*r_val - 0.368f*g_val - 0.071f*b_val;

                // Clip values to YCbCr range
                clip_f(&y, Y_MAX_VAL);
                clip_f(&cb, C_MAX_VAL);
                clip_f(&cr, C_MAX_VAL);

                *r = y;
                *g = cb;
                *b = cr;
            }

            // Write luma value to output array
            ycc_data[ycc_y_idx] = *r;

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
       }
    }
}