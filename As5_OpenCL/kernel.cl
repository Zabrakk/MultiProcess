// Defining the sampler
const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
						  CLK_ADDRESS_NONE	  |
						  CLK_FILTER_NEAREST;

__kernel void resize_and_grayscale(__read_only image2d_t src, __write_only image2d_t dst) {
	// Reduce given images size by a factor of four
	
	// The old width and height have been provided with the source image
	//int w = get_image_width(src); // 735
	
	// Get current coordinates
	int x = get_global_id(0);
	int y = get_global_id(1);
	// Coordinates for every fourth pixel in the source image
	int2 coords_src = {x*4, y*4};
	// Obtain R,G,B,A value from the coordinate
	uint4 values = read_imageui(src, sampler, coords_src);
	
	// Grayscale
	uint gray_val = (uint) (values.s0*0.299 + values.s1*0.587 + values.s2*0.114);

	printf("%d %d\n", x, y);
	
	int2 coord_dst = {x, y};
	write_imageui(dst, coord_dst, gray_val);
	
	// Write new value
	//dst[y*735+x] = convert_uchar(gray_val);
	
}


						  
						  
__kernel void to_grayscale(__global const unsigned char* img, __global unsigned char* result) {
	// Source: "'Standard' RGB to Grayscale Conversion" - https://stackoverflow.com/questions/17615963/standard-rgb-to-grayscale-conversion
	// Turning the image into grayscale by summing red, green, and blue while applying different weights to each one
	
	int id = get_global_id(0);
	//           Red               Green               Blue
	result[id] = img[id*3]*0.299 + img[id*3+1]*0.587 + img[id*3+2]*0.114;
}