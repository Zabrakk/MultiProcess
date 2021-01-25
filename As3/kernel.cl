__kernel void to_grayscale(__global const unsigned char* img, __global unsigned char* result) {
	int id = get_global_id(0);
	// Source: "'Standard' RGB to Grayscale Conversion" - https://stackoverflow.com/questions/17615963/standard-rgb-to-grayscale-conversion
	// Turning the image into grayscale by summing red, green, and blue while applying different weights to each one
	//           Red               Green               Blue
	result[id] = img[id*3]*0.299 + img[id*3+1]*0.587 + img[id*3+2]*0.114;
}

__kernel void apply_mask(__global const unsigned char* img) {

}