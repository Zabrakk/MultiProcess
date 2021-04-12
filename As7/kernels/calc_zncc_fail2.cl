__kernel void calc_zncc(__global const unsigned char* img_left, __global const unsigned char* img_right, __global unsigned char* dst, unsigned int w, unsigned int h, int min_disparity, int max_disparity) {
	// Calculates ZNCC between two given images
	
	// Previously took 1525.946368 milliseconds
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	int thread_num = get_local_id(2);
	
	int window_y = 13, window_x = 11;
	int window_size = window_y * window_x;
	int d, win_y, win_x;
	float max_sum = -1, best_disparity = max_disparity;
	
	if (x < win_x/2 || y < win_y/2 || x+win_x/2 >= w || y+win_y/2 >= h) return;
	
	//printf("HAAH\n");
	
	__local float lw_mean, rw_mean, lower_sum_0, lower_sum_1;
	__local float lw_mean_diffs[12*10], rw_mean_diffs[12*10];
	
	for (d = min_disparity; d < max_disparity; d++) {
		if (thread_num == 0) {
			// Calculate left image values
			for (win_y = -window_y / 2; win_y < window_y / 2; win_y++) {
				for (win_x = -window_x / 2; win_x < window_x / 2; win_x++) {
					// Add current pixel value
					lw_mean += img_left[(win_y + y) * w + (win_x + x)];
				}
			}
			lw_mean = lw_mean / window_size;
			
		} else {
			// Calculate right image values
			for (win_y = -window_y / 2; win_y < window_y / 2; win_y++) {
				for (win_x = -window_x / 2; win_x < window_x / 2; win_x++) {
					// Add current pixel value
					rw_mean += img_right[(win_y + y) * w + (win_x + x - d)];
				}
			}
			rw_mean = rw_mean / window_size;
		}
		barrier(CLK_LOCAL_MEM_FENCE); 
		
		float lw_mean_diff, rw_mean_diff;
		int counter = 0;
		for (win_y = -window_y / 2; win_y < window_y / 2; win_y++) {
			for (win_x = -window_x / 2; win_x < window_x / 2; win_x++) {
				if (thread_num == 0) {
					lw_mean_diff = img_left[(win_y + y) * w + (win_x + x)] - lw_mean;
					lower_sum_0 += lw_mean_diff * lw_mean_diff;
					lw_mean_diffs[counter] = lw_mean_diff; 
					counter += 1;
				} else {
					rw_mean_diff = img_right[(win_y + y) * w + (win_x + x - d)] - rw_mean;
					lower_sum_1 += rw_mean_diff * rw_mean_diff;
					rw_mean_diffs[counter] = rw_mean_diff;
					counter += 1;
				}
			}
		}
		barrier(CLK_LOCAL_MEM_FENCE); 
		
		if (thread_num == 0) {
			float upper_sum = 0.0;
			for(int i = 0; i < 12*10; i++) {
				upper_sum += lw_mean_diffs[i] + rw_mean_diffs[i];
			}
			
			float zncc_val = upper_sum / (sqrt(lower_sum_0) * sqrt(lower_sum_1));
			if (zncc_val > max_sum) {
				best_disparity = d;
				max_sum = zncc_val;
			}
		}
	}
	
	barrier(CLK_LOCAL_MEM_FENCE); 
	// Add resulting best disparity value to the disparity map
	if (thread_num == 0) {
		dst[y * w + x] = convert_uchar(abs((int)best_disparity)); // Use absolute value of the disparity
	}
}
