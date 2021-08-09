#include <arm_neon.h>
#include <stdio.h>
#include <string.h>

#include "cc.h"

// Vectorized RGB->YCC pixel conversion (16 bits). Also writes converted luma pixels to YCC buffer
static void convert_pixels(uint8_t* rgb_data, uint32_t num_pixels, uint8_t* ycc_data) {
    uint8x16x3_t buff;

    int16x8x3_t rgb;
    int16x8x3_t temp;
    uint8x8_t y;
    uint8x8x2_t c;

    int16x4_t coeff1;
    int16x4_t coeff2;

    coeff1[0] = 33;
    coeff1[1] = 65;
    coeff1[2] = 13;

    coeff1[3] = -19;
    coeff2[0] = -37;
    coeff2[1] = 56;

    coeff2[2] = -47;
    coeff2[3] = -9;

    int16x8_t val_16 = vdupq_n_s16(16);
    int16x8_t val_128 = vdupq_n_s16(128);

    int16x8_t val_y_max = vdupq_n_s16(Y_MAX_VAL);
    int16x8_t val_c_max = vdupq_n_s16(C_MAX_VAL);

    // Pre-fetch RGB pixel data
    __asm__ __volatile__(
    "pld\t[%0]"
    :
    : "r" (rgb_data));

    for (uint32_t pixel = 0; pixel < num_pixels; pixel += 16) {
        buff = vld3q_u8(rgb_data+pixel*3);

        rgb.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(buff.val[0])));
        rgb.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(buff.val[1])));
        rgb.val[2] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(buff.val[2])));

        temp.val[0] = vmulq_lane_s16(rgb.val[0], coeff1, 0);
        temp.val[1] = vmulq_lane_s16(rgb.val[0], coeff1, 3);
        temp.val[2] = vmulq_lane_s16(rgb.val[0], coeff2, 1);

        temp.val[0] = vmlaq_lane_s16(temp.val[0], rgb.val[1], coeff1, 1);
        temp.val[1] = vmlaq_lane_s16(temp.val[1], rgb.val[1], coeff2, 0);
        temp.val[2] = vmlaq_lane_s16(temp.val[2], rgb.val[1], coeff2, 2);

        temp.val[0] = vmlaq_lane_s16(temp.val[0], rgb.val[2], coeff1, 2);
        temp.val[1] = vmlaq_lane_s16(temp.val[1], rgb.val[2], coeff2, 1);
        temp.val[2] = vmlaq_lane_s16(temp.val[2], rgb.val[2], coeff2, 3);

        temp.val[0] = vshrq_n_s16(temp.val[0], 7);
        temp.val[1] = vshrq_n_s16(temp.val[1], 7);
        temp.val[2] = vshrq_n_s16(temp.val[2], 7);

        temp.val[0] = vaddq_s16(temp.val[0], val_16);
        temp.val[1] = vaddq_s16(temp.val[1], val_128);
        temp.val[2] = vaddq_s16(temp.val[2], val_128);

        temp.val[0] = vminq_s16(temp.val[0], val_y_max);
        temp.val[1] = vminq_s16(temp.val[1], val_c_max);
        temp.val[2] = vminq_s16(temp.val[2], val_c_max);

        temp.val[0] = vmaxq_s16(temp.val[0], val_16);    
        temp.val[1] = vmaxq_s16(temp.val[1], val_16);
        temp.val[2] = vmaxq_s16(temp.val[2], val_16);

        y = vreinterpret_u8_s8(vmovn_s16(temp.val[0]));
        c.val[0] = vreinterpret_u8_s8(vmovn_s16(temp.val[1]));
        c.val[1] = vreinterpret_u8_s8(vmovn_s16(temp.val[2]));

        vst2_u8(rgb_data+pixel*2, c);        
        vst1_u8(ycc_data+pixel, y);

        rgb.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(buff.val[0])));
        rgb.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(buff.val[1])));
        rgb.val[2] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(buff.val[2])));

        temp.val[0] = vmulq_lane_s16(rgb.val[0], coeff1, 0);
        temp.val[1] = vmulq_lane_s16(rgb.val[0], coeff1, 3);
        temp.val[2] = vmulq_lane_s16(rgb.val[0], coeff2, 1);

        temp.val[0] = vmlaq_lane_s16(temp.val[0], rgb.val[1], coeff1, 1);
        temp.val[1] = vmlaq_lane_s16(temp.val[1], rgb.val[1], coeff2, 0);
        temp.val[2] = vmlaq_lane_s16(temp.val[2], rgb.val[1], coeff2, 2);

        temp.val[0] = vmlaq_lane_s16(temp.val[0], rgb.val[2], coeff1, 2);
        temp.val[1] = vmlaq_lane_s16(temp.val[1], rgb.val[2], coeff2, 1);
        temp.val[2] = vmlaq_lane_s16(temp.val[2], rgb.val[2], coeff2, 3);

        temp.val[0] = vshrq_n_s16(temp.val[0], 7);
        temp.val[1] = vshrq_n_s16(temp.val[1], 7);
        temp.val[2] = vshrq_n_s16(temp.val[2], 7);

        temp.val[0] = vaddq_s16(temp.val[0], val_16);
        temp.val[1] = vaddq_s16(temp.val[1], val_128);
        temp.val[2] = vaddq_s16(temp.val[2], val_128);

        temp.val[0] = vminq_s16(temp.val[0], val_y_max);
        temp.val[1] = vminq_s16(temp.val[1], val_c_max);
        temp.val[2] = vminq_s16(temp.val[2], val_c_max);

        temp.val[0] = vmaxq_s16(temp.val[0], val_16);    
        temp.val[1] = vmaxq_s16(temp.val[1], val_16);
        temp.val[2] = vmaxq_s16(temp.val[2], val_16);

        y = vreinterpret_u8_s8(vmovn_s16(temp.val[0]));
        c.val[0] = vreinterpret_u8_s8(vmovn_s16(temp.val[1]));
        c.val[1] = vreinterpret_u8_s8(vmovn_s16(temp.val[2]));

        vst2_u8(rgb_data+(pixel+8)*2, c);        
        vst1_u8(ycc_data+pixel+8, y);
    }
}

// Downsample converted chroma pixels and write to YCC image buffer
static void downsample_pixels(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    uint8x16x2_t row_top, row_bottom;
    uint16x8_t pairs_top, pairs_bottom, pairs_sum;

    // Downsample converted pixels
    for (uint32_t row = 0; row < rgb_height; row += 2) {
        for (uint32_t col = 0; col < rgb_width; col += 16) {
            // Base source pixel
            const uint32_t pixel_top = (col + (row * rgb_width));
            const uint32_t pixel_bottom = (col + ((row+1) * rgb_width));

            // Destination chroma sections
            const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
            const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

            // Load YCbCr values for rows
            row_top = vld2q_u8(rgb_data + pixel_top*2);
            row_bottom = vld2q_u8(rgb_data + pixel_bottom*2);

            // Downsample Cb values
            pairs_top = vpaddlq_u8(row_top.val[0]);
            pairs_bottom = vpaddlq_u8(row_bottom.val[0]);
            pairs_sum = vaddq_u16(pairs_top, pairs_bottom);
            pairs_sum = vshrq_n_u16(pairs_sum, 2);
            // Store downsampled Cb values
            vst1_u8(ycc_data + ycc_cb_idx, vmovn_u16(pairs_sum));

            // Downsample Cr values
            pairs_top = vpaddlq_u8(row_top.val[1]);
            pairs_bottom = vpaddlq_u8(row_bottom.val[1]);
            pairs_sum = vaddq_u16(pairs_top, pairs_bottom);
            pairs_sum = vshrq_n_u16(pairs_sum, 2);
            // Store downsampled Cr values
            vst1_u8(ycc_data + ycc_cr_idx, vmovn_u16(pairs_sum));
        }
    }
}

static void convert_and_downsample_pixels(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    uint8x8x3_t buff;
    int16x8x3_t rgb;

    int16x8_t temp_y;
    int16x8x2_t c_top, c_bottom;
    uint16x4x2_t c_result;
    uint8_t cb_cr_buffer[8];

    int16x4_t coeff1;
    int16x4_t coeff2;

    coeff1[0] = 33;
    coeff1[1] = 65;
    coeff1[2] = 13;

    coeff1[3] = -19;
    coeff2[0] = -37;
    coeff2[1] = 56;

    coeff2[2] = -47;
    coeff2[3] = -9;

    int16x8_t val_16 = vdupq_n_s16(16);
    int16x8_t val_128 = vdupq_n_s16(128);

    int16x8_t val_y_max = vdupq_n_s16(Y_MAX_VAL);
    int16x8_t val_c_max = vdupq_n_s16(C_MAX_VAL);

    // Downsample converted pixels
    for (uint32_t row = 0; row < rgb_height; row += 2) {
        for (uint32_t col = 0; col < rgb_width; col += 8) {
            // Base source pixel
            const uint32_t pixel_top = (col + (row * rgb_width));
            const uint32_t pixel_bottom = (col + ((row+1) * rgb_width));

            // Destination chroma sections
            const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
            const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

            // Top row RGB->YCC calculation
            buff = vld3_u8(rgb_data + pixel_top*3);
            rgb.val[0] = vreinterpretq_s16_u16(vmovl_u8(buff.val[0]));
            rgb.val[1] = vreinterpretq_s16_u16(vmovl_u8(buff.val[1]));
            rgb.val[2] = vreinterpretq_s16_u16(vmovl_u8(buff.val[2]));

            temp_y = vmulq_lane_s16(rgb.val[0], coeff1, 0);
            c_top.val[0] = vmulq_lane_s16(rgb.val[0], coeff1, 3);
            c_top.val[1] = vmulq_lane_s16(rgb.val[0], coeff2, 1);

            temp_y = vmlaq_lane_s16(temp_y, rgb.val[1], coeff1, 1);
            c_top.val[0] = vmlaq_lane_s16(c_top.val[0], rgb.val[1], coeff2, 0);
            c_top.val[1] = vmlaq_lane_s16(c_top.val[1], rgb.val[1], coeff2, 2);

            temp_y = vmlaq_lane_s16(temp_y, rgb.val[2], coeff1, 2);
            c_top.val[0] = vmlaq_lane_s16(c_top.val[0], rgb.val[2], coeff2, 1);
            c_top.val[1] = vmlaq_lane_s16(c_top.val[1], rgb.val[2], coeff2, 3);

            temp_y = vshrq_n_s16(temp_y, 7);
            c_top.val[0] = vshrq_n_s16(c_top.val[0], 7);
            c_top.val[1] = vshrq_n_s16(c_top.val[1], 7);

            temp_y = vaddq_s16(temp_y, val_16);
            c_top.val[0] = vaddq_s16(c_top.val[0], val_128);
            c_top.val[1] = vaddq_s16(c_top.val[1], val_128);

            temp_y = vminq_s16(temp_y, val_y_max);
            c_top.val[0] = vminq_s16(c_top.val[0], val_c_max);
            c_top.val[1] = vminq_s16(c_top.val[1], val_c_max);

            temp_y = vmaxq_s16(temp_y, val_16);    
            c_top.val[0] = vmaxq_s16(c_top.val[0], val_16);
            c_top.val[1] = vmaxq_s16(c_top.val[1], val_16);

            // Store top-row luma values
            vst1_u8(ycc_data + pixel_top, vreinterpret_u8_s8(vmovn_s16(temp_y)));

            // Bottom row RGB->YCC calculation
            buff = vld3_u8(rgb_data + pixel_bottom*3);
            rgb.val[0] = vreinterpretq_s16_u16(vmovl_u8(buff.val[0]));
            rgb.val[1] = vreinterpretq_s16_u16(vmovl_u8(buff.val[1]));
            rgb.val[2] = vreinterpretq_s16_u16(vmovl_u8(buff.val[2]));

            temp_y = vmulq_lane_s16(rgb.val[0], coeff1, 0);
            c_bottom.val[0] = vmulq_lane_s16(rgb.val[0], coeff1, 3);
            c_bottom.val[1] = vmulq_lane_s16(rgb.val[0], coeff2, 1);

            temp_y = vmlaq_lane_s16(temp_y, rgb.val[1], coeff1, 1);
            c_bottom.val[0] = vmlaq_lane_s16(c_bottom.val[0], rgb.val[1], coeff2, 0);
            c_bottom.val[1] = vmlaq_lane_s16(c_bottom.val[1], rgb.val[1], coeff2, 2);

            temp_y = vmlaq_lane_s16(temp_y, rgb.val[2], coeff1, 2);
            c_bottom.val[0] = vmlaq_lane_s16(c_bottom.val[0], rgb.val[2], coeff2, 1);
            c_bottom.val[1] = vmlaq_lane_s16(c_bottom.val[1], rgb.val[2], coeff2, 3);

            temp_y = vshrq_n_s16(temp_y, 7);
            c_bottom.val[0] = vshrq_n_s16(c_bottom.val[0], 7);
            c_bottom.val[1] = vshrq_n_s16(c_bottom.val[1], 7);

            temp_y = vaddq_s16(temp_y, val_16);
            c_bottom.val[0] = vaddq_s16(c_bottom.val[0], val_128);
            c_bottom.val[1] = vaddq_s16(c_bottom.val[1], val_128);

            temp_y = vminq_s16(temp_y, val_y_max);
            c_bottom.val[0] = vminq_s16(c_bottom.val[0], val_c_max);
            c_bottom.val[1] = vminq_s16(c_bottom.val[1], val_c_max);

            temp_y = vmaxq_s16(temp_y, val_16);    
            c_bottom.val[0] = vmaxq_s16(c_bottom.val[0], val_16);
            c_bottom.val[1] = vmaxq_s16(c_bottom.val[1], val_16);

            // Store bottom-row luma values
            vst1_u8(ycc_data + pixel_bottom, vreinterpret_u8_s8(vmovn_s16(temp_y)));

            // Downsample Cb/Cr values
            c_result.val[0] = vreinterpret_u16_s16(vadd_s16(vpadd_s16(vget_low_s16(c_top.val[0]),vget_high_s16(c_top.val[0])), 
                vpadd_s16(vget_low_s16(c_bottom.val[0]),vget_high_s16(c_bottom.val[0]))));
            c_result.val[1] = vreinterpret_u16_s16(vadd_s16(vpadd_s16(vget_low_s16(c_top.val[1]),vget_high_s16(c_top.val[1])), 
                vpadd_s16(vget_low_s16(c_bottom.val[1]),vget_high_s16(c_bottom.val[1]))));
            c_result.val[0] = vshr_n_u16(c_result.val[0], 2);
            c_result.val[1] = vshr_n_u16(c_result.val[1], 2);
            
            // Store downsampled Cb/Cr values
            vst1_u8(cb_cr_buffer, vmovn_u16(vcombine_u16(c_result.val[0],c_result.val[1])));
            memcpy(ycc_data + ycc_cb_idx, cb_cr_buffer, 4);
            memcpy(ycc_data + ycc_cr_idx, cb_cr_buffer + 4, 4);
        }
    }
}

void cc_vector(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    convert_pixels(rgb_data, rgb_height*rgb_width, ycc_data);
    downsample_pixels(rgb_data, rgb_width, rgb_height, ycc_data);
}

void cc_vector2(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    convert_and_downsample_pixels(rgb_data, rgb_width, rgb_height, ycc_data);
}
