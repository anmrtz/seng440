#include <iostream>
#include <array>

#include "image.hpp"

extern "C" {
#include "cc.h"
}

static constexpr char YCC_FILENAME[] = "ycc-result.png";

enum CC_IMPL {
    NAIVE,
    VECTOR
};

image perform_cc(const image& rgb_img, CC_IMPL cc_impl) {
    // Calculate size of downsampled YCC image
    image ycc_img(rgb_img.get_width() / 2, rgb_img.get_height() / 2);

    uint8_t* rgb_data = rgb_img.get_pixel_data();
    uint8_t* ycc_data = ycc_img.get_pixel_data();

    const uint32_t rgb_width = rgb_img.get_width();
    const uint32_t rgb_height = rgb_img.get_height();

    switch (cc_impl) {
        case NAIVE:
            // Perform naive RGB->YCC colorspace conversion
            std::cout << "Performing naive conversion\n";
            cc_naive(rgb_data, rgb_width, rgb_height, ycc_data);
        break;
        case VECTOR:
            // Perform ARM NEON vectorized conversion
            std::cout << "Performing ARM NEON vectorized conversion\n";
            cc_vector(rgb_data, rgb_width, rgb_height, ycc_data);
        break;
    }

    std::cout << "Conversion complete\n";
    return ycc_img;
}

int main() {
    // Load starting RGB image
    image rgb_img("test.png");

    auto ycc_img = perform_cc(rgb_img, NAIVE);

    // Write the conversion result to a file
    ycc_img.write_image(YCC_FILENAME);
    std::cout << "YCC image written: " << YCC_FILENAME << '\n';

    return 0;
}
