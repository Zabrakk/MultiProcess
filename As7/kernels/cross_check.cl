__kernel void cross_check(__global const unsigned char* left, __global const unsigned char* right, __global unsigned char* dst, unsigned int w, unsigned int h, int th) {
	// Perform cross check between two images
	// Loop structure from the pure C++ implementation removed
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	int coord = y*w + x;
	
	int current_value = abs( (int) (left[coord]-right[coord]));
	if (current_value > th) {
		dst[coord] = 0;
	} else {
		dst[coord] = right[coord];
	}	
}
