#include <vector>
#include <math.h>
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
	int y, x, d;
	int window_size = window_y * window_x; // Size of the whole window
	int win_y, win_x;

	float lw_mean, rw_mean; // Left and right image mean
	float lw_mean_diff, rw_mean_diff; // Pixel difference from the mean
	float lower_sum_0, lower_sum_1, upper_sum;
	float zncc_val, max_sum, best_disparity;

	timer_struct timer;
	StartTimer(&timer);

	#pragma omp parallel
	for (y = 0; y < h; y++) { // Loop to image height
		for (x = 0; x < w; x++) { // Loop to image width
			// Reset
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
			disparity_map[y * w + x] = abs(best_disparity); // Use absolute value of the disparity
		}
	}
	
	StopTimer(&timer, "ZNCC calculated");
	return disparity_map;
}
