#include <iostream>
#include <array>

#include "image.hpp"

extern "C" {
#include "cc.h"
}

void perform_cc(const image& rgb_img, image& ycc_img) {
    uint8_t* rgb_data = rgb_img.get_pixel_data();
    uint8_t* ycc_data = ycc_img.get_pixel_data();

    const uint32_t rgb_width = rgb_img.get_width();
    const uint32_t rgb_height = rgb_img.get_height();

    cc_naive(rgb_data, rgb_width, rgb_height, ycc_data);
} 

int main() {
    // Load starting RGB image
    image img_rgb("test.png");

    // Calculate size of downsampled YCC image
    const uint32_t ycc_width = img_rgb.get_width() / 2;
    const uint32_t ycc_height = img_rgb.get_height() / 2;
    image img_ycc(ycc_width, ycc_height);

    // Perform naive RGB->YCC colorspace conversion
    perform_cc(img_rgb, img_ycc);

    // Write the conversion result to a file
    img_ycc.write_image("ycc-result.png");

    return 0;
}
