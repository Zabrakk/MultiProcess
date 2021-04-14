const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
						  CLK_ADDRESS_CLAMP_TO_EDGE	  |
						  CLK_FILTER_NEAREST;

__kernel void calc_zncc(__read_only image2d_t img_left,
						__read_only image2d_t img_right,
						__write_only image2d_t dst,
						unsigned int w, unsigned int h,
						int min_disparity, int max_disparity,
						int window_x, int window_y) {
	// Previously took 1525.946368 milliseconds
	
	int x = get_global_id(0);
	int y = get_global_id(1);	
	
	if (y-window_y/2 < 0 || window_y/2 + y >= h || x-window_x/2 < 0 || window_x/2 + x >= w) return;
	
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

				lw_mean += read_imagef(img_left, sampler, (int2){ win_y + y, win_x + x }).x;
				rw_mean += read_imagef(img_right, sampler, (int2){ win_y + y, win_x + x - d }).x;
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

				// Get pixel mean differences for both images
				lw_mean_diff = read_imagef(img_left, sampler, (int2){ win_y + y, win_x + x }).x - lw_mean;
				rw_mean_diff = read_imagef(img_right, sampler, (int2){ win_y + y, win_x + x - d }).x - rw_mean;
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
	
	//best_disparity = abs((float)best_disparity)
	float bd = best_disparity*0.01;
	
	// Add resulting best disparity value to the disparity map
	write_imagef(dst, (int2){x, y}, (float4)(bd,bd,bd,1)); // Use absolute value of the disparity
}
