// Defining the sampler
const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
						  CLK_ADDRESS_NONE	  |
						  CLK_FILTER_NEAREST;

						  
__kernel void resize_and_grayscale(__read_only image2d_t src, __write_only image2d_t dst) {
	// Reduce given images size by a factor of four
	// Then grayscale it
	
	// Get current x and y
	int x = get_global_id(0);
	int y = get_global_id(1);
	// Calculate coordinate for the source image
	int2 coords_src = {x*4, y*4};
	// Obtain R,G,B,A value from the coordinate
	uint4 values = read_imageui(src, sampler, coords_src);
	
	// Grayscale the pixel values
	uint gray_val = (uint) (values.s0*0.299 + values.s1*0.587 + values.s2*0.114);
	
	// Write grayscaled value to destination image
	int2 coord_dst = {x, y};
	write_imageui(dst, coord_dst, gray_val);	
}

