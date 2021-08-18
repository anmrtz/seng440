// RGB -> YCbCr 4:2:0 color space converter hardware

typedef struct packed {
	logic [15:0] r, g, b;
} rgb_pixel_t;

typedef struct packed {
	logic [15:0] y, cb, cr;
} ycc_pixel_t;

// Convert RGB values to YCbCr
function ycc_pixel_t rgb_to_ycc(input rgb_pixel_t rgb);
	rgb_to_ycc.y  = 16 + ((33*rgb.r + 65*rgb.g + 13*rgb.b) >> 7);
	rgb_to_ycc.cb = 128 + ((-19*rgb.r - 37*rgb.g + 56*rgb.b) >> 7);
	rgb_to_ycc.cr = 128 + ((56*rgb.r - 47*rgb.g - 9*rgb.b) >> 7);
endfunction : rgb_to_ycc

// Perform 4x4 block downsampling via averaging
function ycc_pixel_t avg4(input ycc_pixel_t ycc_pixels[3:0]);
	avg4.y  = ycc_pixels[0].y;
	avg4.cb = (ycc_pixels[0].cb + ycc_pixels[1].cb + ycc_pixels[2].cb + ycc_pixels[3].cb) >> 2; 
	avg4.cr = (ycc_pixels[0].cr + ycc_pixels[1].cr + ycc_pixels[2].cr + ycc_pixels[3].cr) >> 2;
endfunction : avg4

// Colorspace conversion + downsampling module
module cc_conv(input clk, 
	input logic [23:0] rgb_up_left, input logic [23:0] rgb_up_right, 
	input logic [23:0] rgb_down_left, input logic [23:0] rgb_down_right, 
	output logic [7:0] y_up_left, output logic [7:0] y_up_right, 
	output logic [7:0] y_down_left, output logic [7:0] y_down_right,
	output logic [7:0] cb, output logic [7:0] cr);
	
	always @(posedge clk) begin
		rgb_pixel_t rgb[3:0];
		ycc_pixel_t ycc[3:0];
		ycc_pixel_t avg_ycc;
		
		rgb[0].r = rgb_up_left[7:0];
		rgb[0].g = rgb_up_left[15:8];
		rgb[0].b = rgb_up_left[23:16];

		rgb[1].r = rgb_up_right[7:0];
		rgb[1].g = rgb_up_right[15:8];
		rgb[1].b = rgb_up_right[23:16];

		rgb[2].r = rgb_down_left[7:0];
		rgb[2].g = rgb_down_left[15:8];
		rgb[2].b = rgb_down_left[23:16];

		rgb[3].r = rgb_down_right[7:0];
		rgb[3].g = rgb_down_right[15:8];
		rgb[3].b = rgb_down_right[23:16];
	
		ycc[0] = rgb_to_ycc(rgb[0]);
		ycc[1] = rgb_to_ycc(rgb[1]);
		ycc[2] = rgb_to_ycc(rgb[2]);
		ycc[3] = rgb_to_ycc(rgb[3]);
		
		avg_ycc = avg4(ycc);
		
		y_up_left <= ycc[0].y;
		y_up_right <= ycc[1].y;
		y_down_left <= ycc[2].y;
		y_down_right <= ycc[3].y;
		
		cb <= avg_ycc.y[7:0];
		cr <= avg_ycc.cb[7:0];
	end

endmodule : cc_conv

// Connect colorspace conversion module to parallel-I/O registers
module cc(input clk, 	input logic [23:0] cc_rgb_input_reg_0, input logic [23:0] cc_rgb_input_reg_1, 
	input logic [23:0] cc_rgb_input_reg_2, input logic [23:0] cc_rgb_input_reg_3, 
	output logic [31:0] cc_ycc_output_reg_0, output logic [15:0] cc_ycc_output_reg_1);
	
	cc_conv(clk, cc_rgb_input_reg_0, cc_rgb_input_reg_1, cc_rgb_input_reg_2, cc_rgb_input_reg_3,
		cc_ycc_output_reg_0[7:0], cc_ycc_output_reg_0[15:8], cc_ycc_output_reg_0[23:16], cc_ycc_output_reg_0[31:24], 
		cc_ycc_output_reg_1[7:0], cc_ycc_output_reg_1[15:8]);
		
endmodule : cc
