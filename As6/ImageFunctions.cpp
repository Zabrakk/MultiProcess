#include <vector>
#include <math.h>
#include <algorithm>
#include "lodepng.h"
#include "ImageFunctions.h"
#include "Timer.h"

#include <omp.h> // Include OpenMP. Also enabled in Visual Studio Compiler Settings


void FreeImageVector(std::vector<unsigned char>& img_vector) {
	/* Sources:
	* "How to clear vector in C++ from memory [duplicate]" - https://stackoverflow.com/questions/35514909/how-to-clear-vector-in-c-from-memory
	*/
	img_vector.clear();
	img_vector.shrink_to_fit();
}

int ReadImage(std::vector<unsigned char> &out, const char* filename, unsigned w, unsigned h) {
	printf("Reading image %s with lodepng\n", filename);
	// Initialize a temporary variable for the read image
	unsigned char* temp = NULL;
	timer_struct timer = {};
	
	// Start counting execution time
	StartTimer(&timer);
	if (lodepng_decode32_file(&temp, &w, &h, filename)) {
		// Image loading failed!
		printf("Failed to load the image!\n");
		getchar();
		// Reurn error code 1
		return 1;
	}
	// Convert to std::vector
	out = std::vector<unsigned char>(temp, temp +  w*h*4);
	// Stop counting execution time
	StopTimer(&timer, "Image loaded");
	// According to the lodepng.h, this must be freed
	free(temp);
	// No error occured
	return 0;
}

int WriteImage(std::vector<unsigned char> img, const char* filename, unsigned w, unsigned h, LodePNGColorType type, unsigned bitdepth) {
	printf("Saving %s\n", filename);
	timer_struct timer = {};

	// Start counting execution time
	StartTimer(&timer);
	// Convert img back to unsigned char*
	unsigned char* img_arr = new unsigned char[img.size()];
	std::copy(begin(img), end(img), img_arr);
	// Save the image
	if (lodepng_encode_file(filename, img_arr, w, h, type, bitdepth)) {
		printf("An error occured while saving the image!\n");
		getchar();
		// Return Error code 1
		return 1;
	}
	// Stop counting execution time
	StopTimer(&timer, "Image saved");
	// Free the temporare char array
	delete[] img_arr;
	// No error occured
	return 0;
}

std::vector<unsigned char> ResizeImage(std::vector<unsigned char> img, unsigned int w, unsigned int h) {
	/* Source:
	* "Image scaling and rotating in C/C++" - https://stackoverflow.com/questions/299267/image-scaling-and-rotating-in-c-c
	*/
	timer_struct timer = {};
	// Calculate new width and height
	unsigned new_w = floor(w / 4);
	unsigned new_h = floor(h / 4);
	// Allocate a vector for the new smaller RGBA image
	std::vector<unsigned char> new_img(new_w * new_h * 4);

	StartTimer(&timer);
	// Drop the pixels
	for (int y = 0; y < new_h; y++) {
		for (int x = 0; x < new_w; x++) {
			int dy = y * h / new_h;
			int dx = x * w / new_w;
			int coord = (dx + dy * w) * 4;
			int coord_new = (y * new_w + x) * 4;

			new_img[coord_new] = img[coord];
			new_img[coord_new + 1] = img[coord + 1];
			new_img[coord_new + 2] = img[coord + 2];
			new_img[coord_new + 3] = img[coord + 3];
		}
	}
	StopTimer(&timer, "Image resized");
	return new_img;
}

std::vector<unsigned char> GrayScaleImage(std::vector<unsigned char> img, unsigned int w, unsigned int h) {
	timer_struct timer = {};
	// Initialize a vector for the grayscaled image
	std::vector<unsigned char> grayscaled(w * h); // Resulting image only has one value per pixel
	unsigned int ind = 0; // Counter for current index in resulting image

	StartTimer(&timer);
	// Perform the grayscaling
	for (int i = 0; i < w * h * 4; i += 4) {
		grayscaled[ind] = img[i] * 0.299 + img[i + 1] * 0.587 + img[i + 2] * 0.114;
		ind += 1;
	}
	StopTimer(&timer, "Image grayscaled");
	return grayscaled;
}

std::vector<unsigned char> CalcZNCC(std::vector<unsigned char> img_left, std::vector<unsigned char> img_right, unsigned int w, unsigned int h, int window_y, int window_x, int min_disparity, int max_disparity) {
	std::vector<unsigned char> disparity_map(w * h);
	int window_size = window_y * window_x; // Size of the whole window

	timer_struct timer;
	StartTimer(&timer);

	#pragma omp parallel for num_threads(3)
	for (int y = 0; y < h; y++) { // Loop to image height
		// Declare here so the variables are private for each thread
		int win_y, win_x;
		float lw_mean, rw_mean; // Left and right image mean
		float lw_mean_diff, rw_mean_diff; // Pixel difference from the mean
		float lower_sum_0, lower_sum_1, upper_sum;
		float zncc_val, max_sum, best_disparity;

		for (int x = 0; x < w; x++) { // Loop to image width
			// Reset
			float max_sum = -1; // Start with a small number, so values can update
			float best_disparity = max_disparity;

			for (int d = min_disparity; d < max_disparity; d++) { // Loop to maximum disparity value
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
			disparity_map[y * w + x] = abs(best_disparity); // Use absolute value of the disparity
		}
	}
	StopTimer(&timer, "ZNCC calculated");
	return disparity_map;
}

std::vector<unsigned char> CrossCheck(std::vector<unsigned char> left, std::vector<unsigned char> right, unsigned int w, unsigned int h, unsigned int th) {
	// Allocate memory for the result
	std::vector<unsigned char> result(w * h);
	int current_value;
	// Start the timer
	timer_struct timer;
	
	StartTimer(&timer);
	for (int i = 0; i < w * h; i++) {
		// Compare absolute value of difference between left and right image to the given threshold
		current_value = abs(left[i] - right[i]);
		if (current_value > th) {
			// Threshold exceeded, replacing pixel value with 0
			result[i] = 0;
		}
		else {
			// Add value from right image
			result[i] = right[i];
		}
	}
	// Stop the timer
	StopTimer(&timer, "Cross check done");
	return result;
}

int find_nearest(std::vector<unsigned char> dmap, unsigned int w, unsigned int h, int y, int x) {
	int nh_size = 150;
	int current_val;
	for (int spread = 1; spread <= nh_size / 2; spread++) {
		for (int y_nh = -spread; y_nh <= spread; y_nh++) {
			for (int x_nh = -spread; x_nh <= spread; x_nh++) {
				// Boundary check. Also skip if checking the same coordinate
				if (y_nh + y < 0 || y_nh + y >= h || x_nh + x < 0 || x_nh + x >= w || (y_nh == 0 && x_nh == 0)) {
					continue;
				}
				current_val = dmap[(y + y_nh) * w + (x + x_nh)];
				if (current_val != 0) {
					// Non-zero value found
					return current_val;
				}
			}
		}
	}
	// This allows us to know if the neighborhood is too small, since no non-zero value was hound
	printf("No non-zero neighbor pixel was found!\n");
	return -1;
}

std::vector<unsigned char> OcclusionFill(std::vector<unsigned char> cross, unsigned int w, unsigned int h) {
	std::vector<unsigned char> result(w * h);
	int current_val;

	// Start the timer
	timer_struct timer;

	StartTimer(&timer);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			current_val = cross[y * w + x];
			// Check if current pixel's value is zero
			if (current_val != 0) {
				// Not zero, use the pixels value
				result[y * w + x] = current_val;
			}
			else {
				// Pixel's value is zero, find nearest non zero value in the neighborhood
				current_val = find_nearest(cross, w, h, y, x);
				// Return empty image if no non-zero neighbor was found
				if (current_val == -1) return std::vector<unsigned char>();
				// Assign the new value
				result[y * w + x] = current_val;
			}
		}
	}
	// Stop the timer
	StopTimer(&timer, "Occlusion Fill done");
	return result;
}

std::vector<unsigned char> NormalizeImage(std::vector<unsigned char> dmap, unsigned int w, unsigned int h) {
	/* Sources:
	* "How to Normalize Data Between 0 and 100" - https://www.statology.org/normalize-data-between-0-and-100/
	*/
	timer_struct timer;

	StartTimer(&timer);
	// Get the maximum value of the map
	unsigned int max = *std::max_element(dmap.begin(), dmap.end());
	// Get the minimum value of the map
	unsigned int min = *std::min_element(dmap.begin(), dmap.end());
	// Perform the normalization. Range [0-255]
	for (int i = 0; i < w * h; i++) {
		dmap[i] = 255 * (dmap[i] - min) / (max - min);
	}
	StopTimer(&timer, "Image normalized");
	return dmap;
}
