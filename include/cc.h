#pragma once

#define YCC_MIN_VAL     16
#define Y_MAX_VAL       235
#define C_MAX_VAL       240

extern void cc_naive(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data);
