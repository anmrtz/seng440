#include <stdint.h>

#include "cc.h"

static inline void sat(uint8_t* val, uint8_t max) {
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

void cc_naive(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    // Convert pixel values from RGB to YCC
    for (uint32_t row = 0; row < rgb_height; ++row) {
        for (uint32_t col = 0; col < rgb_width; ++col) {
            const uint32_t idx = 3 * (col + (row * rgb_width));

            uint8_t* r = rgb_data+(idx);
            uint8_t* g = rgb_data+(idx+1);
            uint8_t* b = rgb_data+(idx+2);                

            // Convert RGB values to YCC
            {
                const uint8_t r_val = *r;
                const uint8_t g_val = *g;
                const uint8_t b_val = *b;

                const uint8_t y  =  16 + (((r_val<<6)+(r_val<<1)+(g_val<<7)+g_val+(b_val<<4)+(b_val<<3)+b_val) >> 8);
                const uint8_t cb = 128 + ((-((r_val<<5)+(r_val<<2)+(r_val<<1))-((g_val<<6)+(g_val<<3)+(g_val<<1))+(b_val<<7)-(b_val<<4)) >> 8);
                const uint8_t cr = 128 + (((r_val<<7)-(r_val<<4)-((g_val<<6)+(g_val<<5)-(g_val<<1))-((b_val<<4)+(b_val<<1))) >> 8);

                *r = y;
                *g = cb;
                *b = cr;
            }

            // Saturate values
            sat(r, Y_MAX_VAL);
            sat(g, C_MAX_VAL);
            sat(b, C_MAX_VAL);

            // Process 4x4 clusters for downsampling
            if ((row % 2 == 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;
                const uint32_t up_idx = idx - (rgb_width*3);
                const uint32_t up_left_idx = (idx - 3) - (rgb_width*3);

                *g = avg4(*g, rgb_data[left_idx+1], rgb_data[up_idx+1], rgb_data[up_left_idx+1]);
                *b = avg4(*b, rgb_data[left_idx+2], rgb_data[up_idx+2], rgb_data[up_left_idx+2]);

                // Write YCC values to output image buffer
                const uint32_t ycc_idx = 3*((col >> 1) + ((row >> 1) * (rgb_width >> 1)));
                ycc_data[ycc_idx] = *r;   // Y
                ycc_data[ycc_idx+1] = *g; // Cb
                ycc_data[ycc_idx+2] = *b; // Cr
            }
            else if ((row == rgb_height - 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;

                *g = avg2(*g, rgb_data[left_idx+1]);
                *b = avg2(*b, rgb_data[left_idx+2]);

                // Write YCC values to output image buffer
                const uint32_t ycc_idx = 3*((col >> 1) + ((row >> 1) * (rgb_width >> 1)));
                ycc_data[ycc_idx] = *r;   // Y
                ycc_data[ycc_idx+1] = *g; // Cb
                ycc_data[ycc_idx+2] = *b; // Cr
            }
            else if ((col == rgb_width - 1) && (row % 2 == 1)) {
                const uint32_t up_idx = idx - (rgb_width*3);

                *g = avg2(*g, rgb_data[up_idx+1]);
                *b = avg2(*b, rgb_data[up_idx+2]);

                // Write YCC values to output image buffer
                const uint32_t ycc_idx = 3*((col >> 1) + ((row >> 1) * (rgb_width >> 1)));
                ycc_data[ycc_idx] = *r;   // Y
                ycc_data[ycc_idx+1] = *g; // Cb
                ycc_data[ycc_idx+2] = *b; // Cr
            }
        }
    }
}