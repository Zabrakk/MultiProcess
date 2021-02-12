#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <math.h>
#include <Windows.h>
#include "lodepng.h"
#include "ImageFunctions.h"


void stopTimer(timer_struct timer, std::string action) {
	/* Sources:
	* "Acquiring high-resolution time stamps" - https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
	*/

	QueryPerformanceCounter(&timer.end);
	timer.elapsed.QuadPart = timer.end.QuadPart - timer.start.QuadPart;
	timer.elapsed.QuadPart *= 1000000;
	timer.elapsed.QuadPart /= timer.freq.QuadPart;
	printf("%s Took %ld microseconds\n\n", action.c_str(), timer.elapsed);
}

std::vector<unsigned char> ReadImage(std::string filename, unsigned int w, unsigned int h) {
	/* Sources:
	* LodePNG Decode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
	*/
	std::vector<unsigned char> img;
	printf("Reading image %s with lodepng\n", filename.c_str());

	timer_struct timer;
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);

	if (lodepng::decode(img, w, h, filename, LCT_RGBA) != 0) {
		printf("Failed to load the image!\n");
		getchar();
		return img;
	}
	std::string action = "Image " + filename + " loaded.";
	stopTimer(timer, action);
	return img;
}

bool WriteImage(std::string filename, std::vector<unsigned char> img, unsigned int w, unsigned int h, LodePNGColorType type) {
	/* Sources:
	* LodePNG Encode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_encode.cpp
	*/
	printf("Saving image to %s\n", filename.c_str());
	timer_struct timer;
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);

	unsigned err = lodepng::encode(filename, img, w, h, type);
	if (err) {
		printf("An error occured while saving the image!\n");
		getchar();
		return false;
	}
	std::string action = "Image " + filename + " saved.";
	stopTimer(timer, action);
	return true;
}

std::vector<unsigned char> ResizeImage(std::vector<unsigned char> img, unsigned int w, unsigned int h, unsigned int& new_w, unsigned int& new_h) {
	/* Source:
	* "Image scaling and rotating in C/C++" - https://stackoverflow.com/questions/299267/image-scaling-and-rotating-in-c-c
	*/
	// Calculate new width and height
	new_w = floor(w / 4);
	new_h = floor(h / 4);
	// Allocate a vector for the new image
	std::vector<unsigned char> new_img(new_w * new_h * 4);
	printf("Downscaling image by 4\n");
	printf("New dimetions: width = %d, height = %d\n", new_w, new_h);

	// Start the timer
	timer_struct timer;
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);

	// Drop the pixels
	for (int y = 0; y < new_h; y++) {
		for (int x = 0; x < new_w; x++) {
			int dy = y * h / new_h;
			int dx = x * w / new_w;
			int coord = (dx + dy*w)*4;
			int coord_new = (y * new_w + x)*4;
			printf("");

			new_img[coord_new] = img[coord];
			new_img[coord_new + 1] = img[coord + 1];
			new_img[coord_new + 2] = img[coord + 2];
			new_img[coord_new + 3] = img[coord + 3];
		}
	}
	// Stop the timer
	std::string action = "Image downscaled.";
	stopTimer(timer, action);

	return new_img;
}

std::vector<unsigned char> GrayScaleImage(std::vector<unsigned char> img, unsigned int w, unsigned int h) {
	std::vector<unsigned char> grayscaled(w * h); // Resulting image only has one value per pixel
	unsigned int ind = 0; // Counter for current index in resulting image
	printf("Grayscaling image\n");
	timer_struct timer;
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);

	for (int i = 0; i < w * h * 4; i+=4) {
		grayscaled[ind] = img[i]*0.299 + img[i + 1]*0.587 + img[i + 2]*0.114;
		ind += 1;
	}
	std::string action = "Image grayscaled.";
	stopTimer(timer, action);
	return grayscaled;
}