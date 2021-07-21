#include <stdint.h>

#include "cc.h"

#ifdef CONV_32_BIT
static inline void clip(int32_t* val, int32_t max) {
    if (*val < YCC_MIN_VAL)
        *val = YCC_MIN_VAL;
    else if (*val > max)
        *val = max;
}
#else
static inline void clip(int16_t* val, int16_t max) {
    if (*val < YCC_MIN_VAL)
        *val = YCC_MIN_VAL;
    else if (*val > max)
        *val = max;
}
#endif

static inline uint8_t avg4(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) {
    return (e1 + e2 + e3 + e4) >> 2;
}

void cc_fixed(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    // Convert pixel values from RGB to YCC
    uint32_t ycc_y_idx = 0;
    for (uint32_t row = 0; row < rgb_height; ++row) {
        for (uint32_t col = 0; col < rgb_width; ++col) {
            const uint32_t idx = 3 * ycc_y_idx;

            uint8_t* r = rgb_data+(idx);
            uint8_t* g = rgb_data+(idx+1);
            uint8_t* b = rgb_data+(idx+2);                

            // Convert RGB values to YCC
            {
#ifdef CONV_32_BIT
                const int32_t r_val = *r;
                const int32_t g_val = *g;
                const int32_t b_val = *b;

                int32_t y  =  16 + ((66*r_val + 129*g_val + 25*b_val) >> 8);
                int32_t cb = 128 + ((-38*r_val - 74*g_val + 112*b_val) >> 8);
                int32_t cr = 128 + ((112*r_val - 94*g_val - 18*b_val) >> 8);
#else
                const int16_t r_val = *r;
                const int16_t g_val = *g;
                const int16_t b_val = *b;

                int16_t y  =  16 + ((33*r_val + 65*g_val + 13*b_val) >> 7);
                int16_t cb = 128 + ((-19*r_val - 37*g_val + 56*b_val) >> 7);
                int16_t cr = 128 + ((56*r_val - 47*g_val - 9*b_val) >> 7);
#endif
                // Clip values to YCbCr range
                clip(&y, Y_MAX_VAL);
                clip(&cb, C_MAX_VAL);
                clip(&cr, C_MAX_VAL);

                *r = y;
                *g = cb;
                *b = cr;
            }
            // Write luma value to output array
            ycc_data[ycc_y_idx] = *r;

            ++ycc_y_idx;
        }
    }

    for (uint32_t row = 1; row < rgb_height; row += 2) {
        for (uint32_t col = 1; col < rgb_width; col += 2) {
            const uint32_t ycc_y_idx = (col + (row * rgb_width));
            const uint32_t idx = 3 * ycc_y_idx;

            uint8_t* g = rgb_data+(idx+1);
            uint8_t* b = rgb_data+(idx+2);                

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
