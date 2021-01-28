__kernel void to_grayscale(__global const unsigned char* img, __global unsigned char* result) {
	int id = get_global_id(0);
	// Source: "'Standard' RGB to Grayscale Conversion" - https://stackoverflow.com/questions/17615963/standard-rgb-to-grayscale-conversion
	// Turning the image into grayscale by summing red, green, and blue while applying different weights to each one
	
	//           Red               Green               Blue
	result[id] = img[id*3]*0.299 + img[id*3+1]*0.587 + img[id*3+2]*0.114;
}

__kernel void apply_mask(__global const unsigned char* img, __global unsigned char* result, const int width, const int height) {
	// Error code -5 comes from out of bounds memory access
	
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	int current_y = 0;
	int current_x = 0;
	float sum = 0;
	
	for(int i = -2; i <= 2; i++) {
		current_y = y+i;
		for(int j = -2; j <= 2; j++){
			current_x = x+j;
			
			
			if(current_y >= 0 && current_y < height && current_x >= 0 && current_x < width) {
				// Current position is inside the image
				sum += img[current_y*width + current_x];
				
			} else if((current_y < 0 && current_x < 0) || (current_y >= height && current_x >= width)) {
				// x and y outside of the image, mirror both
				sum += img[(y-i)*width + (x-j)];
				
			} else if((current_y >= 0 && current_y < height) && (current_x < 0 || current_x >= width)) {
				// y is correct, mirror x
				sum += img[current_y*width + (x-j)];
				
			} else if((current_x >= 0 && current_x < width) && (current_y < 0 || current_y >= height)) {
				// x is correct, mirror y
				sum += img[(y-i)*width + current_x];
				
			} else {
				// WHAT THE FUCK?!?!?! SHOULD NEVER GO HERE
				//printf("y: %d, x: %d, mirror value: %d, non mirrored: %d\n", current_y, current_x, img[(y-i)*width + (x-j)], img[(y+i)*width + (x+j)]);
				
				sum += img[(y-i)*width + (x-j)];
				
			}
		}
	}
	
	result[y * width + x] =  (sum / 25);
}




