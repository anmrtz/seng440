#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <tuple>

#include "../include/image.hpp"

extern "C" {
#include "../include/cc.h"
}

enum CC_IMPL {
    NAIVE,
    VECTOR
};

static const std::map<CC_IMPL, std::tuple<std::string, int>> lookup = {
    {NAIVE, {"naive",0}},
    {VECTOR, {"vector",0}},
};

void perform_cc(const image& rgb_img, CC_IMPL cc_impl) {
    const uint32_t rgb_width = rgb_img.get_width();
    const uint32_t rgb_height = rgb_img.get_height();
    uint8_t* rgb_data = rgb_img.get_pixel_data();
    
    std::vector<uint8_t> planar_ycc_data(rgb_img.get_downsampled_size());
    
    switch (cc_impl) {
        case NAIVE:
            // Perform naive RGB->YCC colorspace conversion
            std::cout << "Performing naive conversion\n";
            cc_naive(rgb_data, rgb_width, rgb_height, planar_ycc_data.data());
        break;
        case VECTOR:
            // Perform ARM NEON vectorized conversion
            std::cout << "Performing ARM NEON vectorized conversion\n";
            cc_vector(rgb_data, rgb_width, rgb_height, planar_ycc_data.data());
        break;
    }

//    std::ofstream ycc_file{"ycc_result_" + std::get<0>(lookup.at(cc_impl)) + ".raw", std::ios_base::binary};
    std::ofstream ycc_file{"ycc_result.raw", std::ios_base::binary};
    ycc_file.write((char *)planar_ycc_data.data(), planar_ycc_data.size());

    std::cout << "Conversion complete\n";
}

int main() {
    // Load starting RGB image
    image rgb_img("test.png");

    perform_cc(rgb_img, VECTOR);

    return 0;
}
