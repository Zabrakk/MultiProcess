__kernel void calc_zncc(__global const unsigned char* img_left, __global const unsigned char* img_right, __global unsigned char* dst, unsigned int w, unsigned int h, int min_disparity, int max_disparity) {
	// Calculates ZNCC between two given images
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int window_y = 13, window_x = 11;
	int window_size = window_y * window_x;
	int d, win_y, win_x;
	
	float lw_mean, rw_mean; // Left and right image mean
	float lw_mean_diff, rw_mean_diff; // Pixel difference from the mean
	float lower_sum_0, lower_sum_1, upper_sum;
	float zncc_val, max_sum, best_disparity;
	
	max_sum = -1; // Start with a small number, so values can update
	best_disparity = max_disparity;
	
	for (d = min_disparity; d < max_disparity; d++) { // Loop to maximum disparity value
		// Reset
		lw_mean = 0, rw_mean = 0;
		// Mean for each window. Based on the equation, window_x & window_y should be divided by 2
		for (win_y = -window_y / 2; win_y < window_y / 2; win_y++) {
			for (win_x = -window_x / 2; win_x < window_x / 2; win_x++) {
				// Make sure we are inside the image boundries
				if (win_y + y < 0 || win_y + y >= h || win_x + x < 0 || win_x + x - d < 0 || win_x + x >= w || win_x + x - d >= w) {
					// Outside of image, go to next iteration
					continue;
				}
				// Add current pixel value
				lw_mean += img_left[(win_y + y) * w + (win_x + x)];
				rw_mean += img_right[(win_y + y) * w + (win_x + x - d)];
			}
		}
		// Calculate the window means by dividing summed values with the window's size
		lw_mean = lw_mean / window_size;
		rw_mean = rw_mean / window_size;

		//Reset
		upper_sum = 0, lower_sum_0 = 0, lower_sum_1 = 0, zncc_val = 0;

		// Calculate ZNCC using the same window loops
		for (win_y = -window_y / 2; win_y < window_y / 2; win_y++) {
			for (win_x = -window_x / 2; win_x < window_x / 2; win_x++) {
				// Make sure we are inside the image boundries
				if (win_y + y < 0 || win_y + y >= h || win_x + x < 0 || win_x + x - d < 0 || win_x + x >= w || win_x + x - d >= w) {
					// Outside of image, go to next iteration
					continue;
				}
				// Get pixel mean differences for both images
				lw_mean_diff = img_left[(win_y + y) * w + (win_x + x)] - lw_mean;
				rw_mean_diff = img_right[(win_y + y) * w + (win_x + x - d)] - rw_mean;
				// Lower Sum calculation
				lower_sum_0 += lw_mean_diff * lw_mean_diff;
				lower_sum_1 += rw_mean_diff * rw_mean_diff;
				// Upper Sum calculation
				upper_sum += lw_mean_diff * rw_mean_diff;
			}
		}
		// Calculating the ZNCC value with upper and lower sum
		zncc_val = upper_sum / (sqrt(lower_sum_0) * sqrt(lower_sum_1));
		// Check if maximum sum and best disparity should be updated based on current zncc value
		if (zncc_val > max_sum) {
			best_disparity = d;
			max_sum = zncc_val;
		}
	}
	// Add resulting best disparity value to the disparity map
	dst[y * w + x] = convert_uchar(abs((int)best_disparity)); // Use absolute value of the disparity
}
