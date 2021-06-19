#include <iostream>
#include <array>

#include "image.hpp"

#define YCC_MIN_VAL     16
#define Y_MAX_VAL       235
#define C_MAX_VAL       240

inline void sat(uint8_t& val, uint8_t max) {
    if (val < YCC_MIN_VAL)
        val = YCC_MIN_VAL;
    else if (val > max)
        val = max;
}

inline uint8_t avg2(uint8_t e1, uint8_t e2) {
    return (e1 + e2) >> 1;
}

inline uint8_t avg4(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) {
    return (e1 + e2 + e3 + e4) >> 2;
}

void cc_naive(const image& rgb_img, image& ycc_img) {
    uint8_t* const rgb_data = rgb_img.get_pixel_data();
    uint8_t* const ycc_data = ycc_img.get_pixel_data();

    const uint32_t rgb_width = rgb_img.get_width();
    const uint32_t rgb_height = rgb_img.get_height();

    // Convert pixel values from RGB to YCC
    for (uint32_t row = 0; row < rgb_height; ++row) {
        for (uint32_t col = 0; col < rgb_width; ++col) {
            const uint32_t idx = 3 * (col + (row * rgb_width));

            uint8_t& r = rgb_data[idx];
            uint8_t& g = rgb_data[idx+1];
            uint8_t& b = rgb_data[idx+2];

            // Convert RGB values to YCC
            const uint8_t temp_y = 16 + (((r<<6)+(r<<1)+(g<<7)+g+(b<<4)+(b<<3)+b)>>8);
            const uint8_t temp_cb = 128 + ((-((r<<5)+(r<<2)+(r<<1))-((g<<6)+(g<<3)+(g<<1))+(b<<7)-(b<<4))>>8);
            const uint8_t temp_cr = 128 + (((r<<7)-(r<<4)-((g<<6)+(g<<5)-(g<<1))-((b<<4)+(b<<1)))>>8);

            r = temp_y;
            g = temp_cb;
            b = temp_cr;

            // Saturate values
            sat(rgb_data[idx], Y_MAX_VAL);
            sat(rgb_data[idx + 1], C_MAX_VAL);
            sat(rgb_data[idx + 2], C_MAX_VAL);

            // Process 4x4 clusters for downsampling
            if ((row % 2 == 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;
                const uint32_t up_idx = idx - (rgb_width*3);
                const uint32_t up_left_idx = (idx - 3) - (rgb_width*3);

                rgb_data[idx+1] = avg4(rgb_data[idx+1], rgb_data[left_idx+1], rgb_data[up_idx+1], rgb_data[up_left_idx+1]);
                rgb_data[idx+2] = avg4(rgb_data[idx+2], rgb_data[left_idx+2], rgb_data[up_idx+2], rgb_data[up_left_idx+2]);

                // Write YCC values to output image buffer
                const uint32_t ycc_idx = 3*((col >> 1) + ((row >> 1) * (rgb_width >> 1)));
                ycc_data[ycc_idx] = rgb_data[idx];
                ycc_data[ycc_idx+1] = rgb_data[idx+1];
                ycc_data[ycc_idx+2] = rgb_data[idx+2];
            }
            else if ((row == rgb_height - 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;

                rgb_data[idx+1] = avg2(rgb_data[idx+1], rgb_data[left_idx+1]);
                rgb_data[idx+2] = avg2(rgb_data[idx+2], rgb_data[left_idx+2]);

                const uint32_t ycc_idx = 3*((col >> 1) + ((row >> 1) * (rgb_width >> 1)));
                ycc_data[ycc_idx] = rgb_data[idx];
                ycc_data[ycc_idx+1] = rgb_data[idx+1];
                ycc_data[ycc_idx+2] = rgb_data[idx+2];
            }
            else if ((col == rgb_width - 1) && (row % 2 == 1)) {
                const uint32_t up_idx = idx - (rgb_width*3);

                rgb_data[idx+1] = avg2(rgb_data[idx+1], rgb_data[up_idx+1]);
                rgb_data[idx+2] = avg2(rgb_data[idx+2], rgb_data[up_idx+2]);
            }
        }
    }
} 

int main() {
    // Load starting RGB image
    image img_rgb("test.png");

    // Calculate size of downsampled YCC image
    const uint32_t ycc_width = img_rgb.get_width() / 2;
    const uint32_t ycc_height = img_rgb.get_height() / 2;
    image img_ycc(ycc_width, ycc_height);

    // Perform naive RGB->YCC colorspace conversion
    cc_naive(img_rgb, img_ycc);

    // Write the conversion result to a file
    img_ycc.write_image("ycc-result.png");

    return 0;
}
