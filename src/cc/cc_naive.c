#include "cc.h"

static void __attribute__ ((noinline)) clip(int16_t* val, int16_t max) {
    if (*val < YCC_MIN_VAL)
        *val = YCC_MIN_VAL;
    else if (*val > max)
        *val = max;
}

static uint8_t __attribute__ ((noinline)) avg4(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) {
    return (e1 + e2 + e3 + e4) >> 2;
}

void cc_naive(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
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
                const int16_t r_val = *r;
                const int16_t g_val = *g;
                const int16_t b_val = *b;

                int16_t y  =  16 + ((33*r_val + 65*g_val + 13*b_val) >> 7);
                int16_t cb = 128 + ((-19*r_val - 37*g_val + 56*b_val) >> 7);
                int16_t cr = 128 + ((56*r_val - 47*g_val - 9*b_val) >> 7);

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
