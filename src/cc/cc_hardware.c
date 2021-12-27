#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cc.h"

// Color conversion hardware parallel IO bridge address
#define BRIDGE 0xC0000000
#define BRIDGE_SPAN 0x18
// 128-bit parallel output offset for RCB data to hardware
#define RGB_BLOCK 0x00
// 64-bit parallel input offset for YCbCr data from hardware
#define CC_RESULT 0x10

extern void cc_hardware(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {    
    // Map parallel IO physical addresses to virtual address space
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Couldn't open /dev/mem\n");
        return;
    }
    uint8_t *bridge = (uint8_t *)mmap(NULL, BRIDGE_SPAN, PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd, BRIDGE);
    close(fd);
    if (bridge == MAP_FAILED) {
        perror("mmap failed.");
        return;
    }

    // RGB hardware input address
    uint8_t *rgb_block = bridge + RGB_BLOCK;
    // YCbCr hardware output address
    uint8_t *cc_result = bridge + CC_RESULT;

    // Intermediate transfer buffers
    uint8_t rgb_inputs[12];
    uint8_t cc_output[6];

    // Convert and downsample pixels
    for (uint32_t row = 0; row < rgb_height; row += 2) {
        for (uint32_t col = 0; col < rgb_width; col += 2) {
            // Base source pixel
            const uint32_t pixel_top = (col + (row * rgb_width));
            const uint32_t pixel_bottom = (col + ((row+1) * rgb_width));

            // Destination chroma sections
            const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
            const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

            // Send 4x4 RGB pixel block to the memory-mapped RGB->YCbCr conversion hardware and allow 1 clock cycle for conversion
            {
                memcpy(rgb_inputs + 0, rgb_data + (pixel_top*3), 3);
                memcpy(rgb_inputs + 3, rgb_data + ((pixel_top+1)*3), 3);
                memcpy(rgb_inputs + 6, rgb_data + (pixel_bottom*3), 3);
                memcpy(rgb_inputs + 9, rgb_data + ((pixel_bottom+1)*3), 3);

                memcpy(rgb_block, rgb_inputs, 12);
            }

            memcpy(cc_output, cc_result, 6);
            // Retrieve and store output data from the hardware YCbCr output registers
            // Store top-row luma values
            *((uint16_t*)(ycc_data + pixel_top)) = *((uint16_t*)cc_output); // 2);
            // Store bottom-row luma values
            *((uint16_t*)(ycc_data + pixel_bottom)) = *((uint16_t*)(cc_output + 2)); //, 2);
            // Store downsampled Cb value
            *(ycc_data + ycc_cb_idx) = *(cc_output + 4); //, 1);
            // Store downsampled Cr value
            *(ycc_data + ycc_cr_idx) = *(cc_output + 5); //, 1);
        }
    }

    munmap(bridge, BRIDGE_SPAN);
}
