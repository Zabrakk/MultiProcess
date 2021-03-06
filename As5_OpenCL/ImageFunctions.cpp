#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <Windows.h>
#include <thread>
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

std::vector<unsigned char> charArrToVector(unsigned char* arr, unsigned int size) {
	/* Sources:
	* "How to convert unsigned char[] to std::vector<unsigned char>" - https://stackoverflow.com/questions/27951537/how-to-convert-unsigned-char-to-stdvectorunsigned-char
	*/
	return std::vector<unsigned char>(arr, arr + size);
}

void FreeImageVector(std::vector<unsigned char>& img_vector) {
	/* Sources:
	* "How to clear vector in C++ from memory [duplicate]" - https://stackoverflow.com/questions/35514909/how-to-clear-vector-in-c-from-memory
	*/
	img_vector.clear();
	img_vector.shrink_to_fit();
}

int ReadImage(unsigned char** out, const char* filename, unsigned w, unsigned h) {
	/* Sources:
	* LodePNG Decode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
	*/
	printf("Reading image %s with lodepng\n", filename);

	timer_struct timer;
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);
	// Read the image
	if (lodepng_decode32_file(out, &w, &h, filename)) {
		printf("Failed to load the image!\n");
		getchar();
		// Return error code 1
		return 1;
	}
	// Stop the timer
	std::string action = "Image " + (std::string)filename + " loaded.";
	stopTimer(timer, action);
	// No error occured
	return 0;
}

int WriteImage(const char* filename, std::vector<unsigned char> img, unsigned int w, unsigned int h, LodePNGColorType type, unsigned bitdepth) {
	/* Sources:
	* "Unsigned char std::vector to unsigned char[]?" - https://stackoverflow.com/questions/3225568/unsigned-char-stdvector-to-unsigned-char
	*/
	printf("Saving image to %s\n", filename);

	timer_struct timer;
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);

	// Convert to unsigned char array
	unsigned char* img_arr = new unsigned char[img.size()];
	std::copy(begin(img), end(img), img_arr);
	// Save the image
	if (lodepng_encode_file(filename, img_arr, w, h, type, bitdepth)) {
		printf("An error occured while saving the image!\n");
		getchar();
		// Return error code 1
		return 1;
	}
	// Stop the timer
	std::string action = "Image " + (std::string)filename + " saved.";
	stopTimer(timer, action);
	// No error occured
	return 0;
}
