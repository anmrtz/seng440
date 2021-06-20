#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>

#include "lodepng.h"

// Wrapper for loaded images
class image {
    public:

    image(const std::string& filename) {
        if (lodepng_decode24_file(&pixel_data,&width,&height,filename.c_str()) != 0) {
            throw std::runtime_error("Failed to load PNG file " + filename);
        }
    }

    image(uint32_t width, uint32_t height)
    : width(width), height(height) {
        pixel_data = (uint8_t*)malloc(width * height * 3);
        if (!pixel_data) {
            throw std::runtime_error("Failed to allocate memory for new image");
        }
    }

    image() = delete;

    // No copying allowed
    image(const image&) = delete;
    image& operator=(const image&) = delete;

    void write_image(const std::string& filename) const {
        if (lodepng_encode24_file(filename.c_str(), pixel_data, width, height) != 0) {
            throw std::runtime_error("Failed to write PNG file " + filename);
        }
    }

    uint32_t get_width() const {
        return width;
    }

    uint32_t get_height() const {
        return height;
    }

    uint8_t* get_pixel_data() const {
        return pixel_data;
    }

    ~image() {
        if (pixel_data != nullptr) {
            free(pixel_data);
        }
    }

    private:

    uint32_t width, height;
    uint8_t* pixel_data = nullptr;
};
