#include <iostream>
#include <array>

#include "image.hpp"

template <typename T>
T sat(T val, T min, T max) {
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

template <typename T>
T avg4(T e1, T e2, T e3, T e4) {
    return (e1 + e2 + e3 + e4) / 4;
}

void cc_naive_float() {}

int main() {
    //image img1("test.png");

    //img1.write_image("test_out.png");

    image img2(512,256);

    uint8_t* const pixels = img2.get_pixels();
    const uint32_t width = img2.get_width();
    const uint32_t height = img2.get_height();

    for (uint32_t col = 0; col < height; ++col) {
        for (uint32_t row = 0; row < width; ++row) {
            const uint32_t idx = 3 * (row + (col * width));
            pixels[idx] = 33;
            pixels[idx + 1] = 66;
            pixels[idx + 2] = 99;
        }
    }

    img2.write_image("sample.png");

    // Convert rgb to ycc array
    // Resize image
    // Output converted image

/*
    float r, g, b, y, cr, cb;
    for(int row = 0; row < 0; ++row) {
        for(int col = 0; col < 0; ++col) {
            y = 16.0 + 0.257 *r + 0.504 *g + 0.098 *b;
            cb = 128.0 - 0.148 *r - 0.291 *g + 0.439 *b;
            cr = 128.0 + 0.439 *r - 0.368 *g - 0.071 *b;
        }
    }
*/

    return 0;
}
