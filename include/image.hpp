#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>

#include "lodepng.h"

// Wrapper for loaded images
class image {
    public:

    image() = delete;
    image(const image&) = delete;
    image& operator=(const image&) = delete;

    image(const std::string& filename) {
        if (lodepng_decode24_file(&pixels,&width,&height,filename.c_str()) != 0) {
            throw std::runtime_error("Failed to load PNG file " + filename);
        }
    }

    image(uint32_t width, uint32_t height)
    : width(width), height(height) {
        pixels = (uint8_t*)malloc(width * height * 3);
        if (!pixels) {
            throw std::runtime_error("Failed to allocate memory for new image");
        }
    }

    void write_image(const std::string& filename) {
        if (lodepng_encode24_file(filename.c_str(), pixels, width, height) != 0) {
            throw std::runtime_error("Failed to write PNG file " + filename);
        }
    }

    uint32_t get_width() const {
        return width;
    }

    uint32_t get_height() const {
        return height;
    }

    uint8_t* get_pixels() const {
        return pixels;
    }

    ~image() {
        if (pixels != nullptr) {
            free(pixels);
        }
    }

    private:

    uint32_t width, height;
    uint8_t* pixels = nullptr;

};
