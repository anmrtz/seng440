#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "lodepng.h"

// Wrapper for loaded images
class image {
    public:

    image(const std::string& filename) {
        if (lodepng_decode24_file(&pixel_data,&width,&height,filename.c_str()) != 0) {
            throw std::runtime_error("Failed to load PNG file " + filename);
        }

        // Pad data arrays to multiples of 24 to ensure compability with vectorization algorithms
        const std::size_t data_size = width*height*3;
        const std::size_t padded_size = get_padded_size();
        if ((data_size) != padded_size) {
            uint8_t* temp_pixel_data = (uint8_t*)calloc(padded_size, 1);
            if (!temp_pixel_data) {
                throw std::runtime_error("Failed to allocate memory for new image");
            }

            std::cout << "Data/padded: " << data_size << " / " << padded_size << '\n';

            std::copy_n(pixel_data, data_size, temp_pixel_data);

            free(pixel_data);
            pixel_data = temp_pixel_data;
        }
    }

    image() = delete;

    // No copying allowed
    image(const image&) = delete;
    image& operator=(const image&) = delete;

    image(image&& other) {
        this->width = other.width;
        this->height = other.height;

        this->pixel_data = other.pixel_data;
        other.pixel_data = nullptr;
    }

    image& operator=(image && other) {
        this->width = other.width;
        this->height = other.height;

        this->pixel_data = other.pixel_data;
        other.pixel_data = nullptr;

        return *this;
    }

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

    std::size_t get_downsampled_size() const {
        return (width*height) + ((width*height + 1) / 2);
    }

    ~image() {
        if (pixel_data != nullptr) {
            free(pixel_data);
        }
    }

    private:

    uint32_t width, height;
    uint8_t* pixel_data = nullptr;

    std::size_t get_padded_size() const {
        const std::size_t num_data_bytes = width*height*3;
        return num_data_bytes + (24 - (num_data_bytes % 24));
    }
};
