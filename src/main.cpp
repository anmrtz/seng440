#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <tuple>
#include <chrono>
#include <string>

#include "../include/image.hpp"

extern "C" {
#include "../include/cc.h"
}

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

enum CC_IMPL {
    NAIVE,
    VECTOR
};

static const std::map<std::string, CC_IMPL> cc_lookup = {
    {"naive", NAIVE},
    {"vector", VECTOR},
};

void perform_cc(const image& rgb_img, CC_IMPL cc_impl) {
    const uint32_t rgb_width = rgb_img.get_width();
    const uint32_t rgb_height = rgb_img.get_height();
    uint8_t* rgb_data = rgb_img.get_pixel_data();
    
    std::vector<uint8_t> planar_ycc_data(rgb_img.get_downsampled_size());
    
    decltype(high_resolution_clock::now()) t1, t2;
    switch (cc_impl) {
        case NAIVE:
            // Perform naive RGB->YCC colorspace conversion
            std::cout << "Performing naive conversion\n";
            t1 = high_resolution_clock::now();
            cc_naive(rgb_data, rgb_width, rgb_height, planar_ycc_data.data());
        break;
        case VECTOR:
            // Perform ARM NEON vectorized conversion
            std::cout << "Performing ARM NEON vectorized conversion\n";
            t1 = high_resolution_clock::now();
            cc_vector(rgb_data, rgb_width, rgb_height, planar_ycc_data.data());
        break;
    }
    t2 = high_resolution_clock::now();
    const duration<double, std::milli> time_ms = t2 - t1;
    std::cout << "Conversion complete. Duration: " << time_ms.count() << " ms\n";

//    std::ofstream ycc_file{"ycc_result_" + std::get<0>(lookup.at(cc_impl)) + ".raw", std::ios_base::binary};
    std::ofstream ycc_file{"ycc-result.raw", std::ios_base::binary};
    ycc_file.write((char *)planar_ycc_data.data(), planar_ycc_data.size());
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Missing arg\n";
        return 1;
    }

    const std::string arg = argv[1];

    const auto cc_type = cc_lookup.find(arg);
    if (cc_type == cc_lookup.end()) {
        std::cout << "Invalid argument: " << arg << '\n';
        return 1;
    }

    // Load starting RGB image
    image rgb_img("large.png");

    perform_cc(rgb_img, cc_type->second);

    return 0;
}
