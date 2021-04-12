__kernel void calc_zncc(__global const unsigned char* img_left, __global const unsigned char* img_right, __global unsigned char* dst, unsigned int w, unsigned int h, int min_disparity, int max_disparity) {
	// Calculates ZNCC between two given images
	
	// Previously took 1525.946368 milliseconds
	
	// Obtain global position in the image
	int2 glob_pos = (int2)(get_global_id(0), get_global_id(1));
	// Obtain local position in the window. These will be used with value -local & local to perform the window usage
	int2 loc_pos = (int2)(get_local_id(0), get_local_id(1));
	// x between 0-5 and y between 0-6
	
	// Get global size
	int2 global_size = (int2)(get_global_size(0), get_global_size(0));
	// Get local size
	int2 local_size = (int2)(get_local_size(0), get_local_size(1));
	
	
	// Initialize other variables
	int window_size = 10*12; // Variable lengths not supported, so use hardcoded instead
	int d, win_y, win_x;
	// Create arrays for local values
	__local unsigned char local_img_left[10*12];
	__local unsigned char local_img_right[10*12];
	
	__local float lw_mean[1], rw_mean[1], lower_sum_0[1], lower_sum_1[1], upper_sum[1], max_sum[1], best_disparity[1], zncc_val[1];
	
	max_sum[0] = -1; // Start with a small number, so values can update
	best_disparity[0] = max_disparity;
	
	// Shortened local coodinates
	int my_pos = loc_pos.y * local_size.x*2 + loc_pos.x;
	int my_pos_neg = (loc_pos.y * local_size.x*2 + loc_pos.x)+local_size.x;

	
	
	for (d = min_disparity; d < max_disparity; d++) { // Loop to maximum disparity value
		// Reset
		lw_mean[0] = 0, rw_mean[0] = 0;
		upper_sum[0] = 0, lower_sum_0[0] = 0, lower_sum_1[0] = 0, zncc_val[0] = 0;
	
		// Store from global memory to local
		// Positive window_x & window_y for left image
		local_img_left[my_pos] = img_left[(glob_pos.y + loc_pos.y) * w + (glob_pos.x + loc_pos.x)];
		// Negative window_x & window_y for left image
		local_img_left[my_pos_neg] = img_left[(glob_pos.y - loc_pos.y) * w + (glob_pos.x - loc_pos.x)];
		
		// Positive window_x & window_y for right image
		local_img_right[my_pos] = img_left[(glob_pos.y + loc_pos.y) * w + (glob_pos.x + loc_pos.x - d)];
		// Negative window_x & window_y for right image
		local_img_right[my_pos_neg] = img_left[(glob_pos.y - loc_pos.y) * w + (glob_pos.x - loc_pos.x - d)];

		barrier(CLK_LOCAL_MEM_FENCE); 
	
		lw_mean[0] += local_img_left[my_pos];
		lw_mean[0] += local_img_left[my_pos_neg];
			
		rw_mean[0] += local_img_right[my_pos];
		rw_mean[0] += local_img_right[my_pos_neg];

		barrier(CLK_LOCAL_MEM_FENCE); 
		
		if (loc_pos.x == 0 && loc_pos.y == 0) {
			// Calculate the window means by dividing summed values with the window's size
			lw_mean[0] = lw_mean[0] / window_size;
			rw_mean[0] = rw_mean[0] / window_size;
			
		}	
		barrier(CLK_LOCAL_MEM_FENCE); 
	
		lower_sum_0[0] += (local_img_left[my_pos]-lw_mean[0]) * (local_img_left[my_pos]-lw_mean[0]);
		lower_sum_0[0] += (local_img_left[my_pos_neg]-lw_mean[0]) * (local_img_left[my_pos_neg]-lw_mean[0]);
		
		lower_sum_1[0] += (local_img_right[my_pos]-rw_mean[0]) * (local_img_right[my_pos]-rw_mean[0]);
		lower_sum_1[0] += (local_img_right[my_pos_neg]-rw_mean[0]) * (local_img_right[my_pos_neg]-rw_mean[0]);
		
		upper_sum[0] += (local_img_left[my_pos]-lw_mean[0]) * (local_img_right[my_pos]-rw_mean[0]);
		upper_sum[0] += (local_img_left[my_pos_neg]-lw_mean[0]) * (local_img_right[my_pos_neg]-rw_mean[0]);
		
		barrier(CLK_LOCAL_MEM_FENCE); 
		if (loc_pos.x == 0 && loc_pos.y == 0) {
			zncc_val[0] = upper_sum[0] / (sqrt(lower_sum_0[0]) * sqrt(lower_sum_1[0]));
			if (zncc_val[0] > max_sum[0]) {
				best_disparity[0] = d;
				max_sum[0] = zncc_val[0];
			}	
		}
		barrier(CLK_LOCAL_MEM_FENCE); 
	}
	barrier(CLK_LOCAL_MEM_FENCE); 
	if (loc_pos.x == 0 && loc_pos.y == 0) {
		dst[glob_pos.y * w + glob_pos.x] = convert_uchar(abs((int)best_disparity[0]));
	}
	
}
