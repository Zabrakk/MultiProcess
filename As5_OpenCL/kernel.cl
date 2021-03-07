// Defining the sampler
const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
						  CLK_ADDRESS_NONE	  |
						  CLK_FILTER_NEAREST;

__kernel void resize_and_grayscale(const unsigned w, const unsigned h, __read_only image2d_t src, __global unsigned char* dst) {
	// Reduce given images size by a factor of four
	
	// The old width and height have been provided with the source image
	//int w = get_image_width(src); // 735
	
	// Get current coordinates
	int x = get_global_id(0);
	int y = get_global_id(1);
	// Coordinates for every fourth pixel in the source image
	int2 coords = {x, y};
	// Obtain R,G,B,A value from the coordinate
	uint4 values = read_imageui(src, sampler, coords);
	
	// Grayscale
	uint gray_val = (uint) (values.s0*0.299 + values.s1*0.587 + values.s2*0.114);

	// Write new value
	dst[y*2940+x] = convert_uchar(gray_val);
	
}


						  
						  
__kernel void to_grayscale(__global const unsigned char* img, __global unsigned char* result) {
	// Source: "'Standard' RGB to Grayscale Conversion" - https://stackoverflow.com/questions/17615963/standard-rgb-to-grayscale-conversion
	// Turning the image into grayscale by summing red, green, and blue while applying different weights to each one
	
	int id = get_global_id(0);
	//           Red               Green               Blue
	result[id] = img[id*3]*0.299 + img[id*3+1]*0.587 + img[id*3+2]*0.114;
}