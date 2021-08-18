#include <string.h>

#include "cc.h"

/* 
 * Simulated 32-bit memory-mapped registers for the 4x4 RGB->YCbCr conversion hardware
 * These registers correspond to the parallel-I/O registers found in the cc.sv hardware description file:
 *
 * cc_rgb_input_reg_0 = hardware_rgb_input_regs[0] // RGB top-left pixel (24 bits used)
 * cc_rgb_input_reg_1 = hardware_rgb_input_regs[1] // RGB top-right pixel (24 bits used)
 * cc_rgb_input_reg_2 = hardware_rgb_input_regs[2] // RGB bottom-left pixel (24 bits used)
 * cc_rgb_input_reg_3 = hardware_rgb_input_regs[3] // RGB bottom-right pixel (24 bits used)
 * 
 * cc_ycc_output_reg_1 = hardware_ycc_output_regs[0] // Output luma (Y) values (32 bits total used for four Y values)
 * cc_ycc_output_reg_1 = hardware_ycc_output_regs[0] // Output chroma (Cb/Cr) values (16 bits total used for Cb + Cr values)
 */
static volatile uint32_t hardware_rgb_input_regs[4];  // 4 RGB pixels = 96 bits
static volatile uint32_t hardware_ycc_output_regs[2]; // 4 Y + 1 Cb + 1 Cr = 48 bits

extern void cc_hardware(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {    
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
                // Write 4x4 RGB data to the hardware RGB input registers
                hardware_rgb_input_regs[0] = *((uint32_t*)(rgb_data + (pixel_top*3)));         // Top-left pixel
                hardware_rgb_input_regs[1] = *((uint32_t*)(rgb_data + ((pixel_top+1)*3)));     // Top-right pixel
                hardware_rgb_input_regs[2] = *((uint32_t*)(rgb_data + (pixel_bottom*3)));      // Bottom-left pixel
                hardware_rgb_input_regs[3] = *((uint32_t*)(rgb_data + ((pixel_bottom+1)*3)));  // Bottom-right pixel
            
                // Hardware takes one clock cycle to complete conversion + downsampling of a 4x4 block once all values are loaded
                // A dummy instruction is used to delay 1 cycle for the hardware operation
                __asm__ __volatile("mov	r0, r0");
            }

            // Retrieve and store output data from the hardware YCbCr output registers
            // Store top-row luma values
            memcpy(ycc_data + pixel_top, (uint8_t*)hardware_ycc_output_regs, 2);
            // Store bottom-row luma values
            memcpy(ycc_data + pixel_bottom, ((uint8_t*)hardware_ycc_output_regs) + 2, 2);
            // Store downsampled Cb value
            memcpy(ycc_data + ycc_cb_idx, ((uint8_t*)hardware_ycc_output_regs) + 4, 1);
            // Store downsampled Cr value
            memcpy(ycc_data + ycc_cr_idx, ((uint8_t*)hardware_ycc_output_regs) + 5, 1);
        }
    }
}
