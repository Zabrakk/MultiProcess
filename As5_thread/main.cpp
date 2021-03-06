#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include <thread>
#include "lodepng.h"
#include "ImageFunctions.h"

#define WINDOW_Y 13 
#define WINDOW_X 11 
#define MIN_DISPARITY 0
#define MAX_DISPARITY 65 // Scaled down. 260/4 as stated in the Assignment
#define THRESHOLD 3

int main() {
	// Image width and height. Both input images are the same size
	unsigned w = 2940;
	unsigned h = 2016;
	// Timing info
	timer_struct timer;

	// Initialize original images
	unsigned char* im0 = NULL;
	unsigned char* im1 = NULL;

	// Read the original images
	if (ReadImage(&im0, "im0.png", w, h)) return 1;
	if (ReadImage(&im1, "im1.png", w, h)) return 1;
	// Convert to a nicer format, i.e. vector
	std::vector<unsigned char> im0_vector = charArrToVector(im0, w * h * 4);
	std::vector<unsigned char> im1_vector = charArrToVector(im1, w * h * 4);
	// Free the unsigned char arrays from memory
	delete im0;
	delete im1;

	// Resize the images
	im0_vector = ResizeImage(im0_vector, w, h);
	im1_vector = ResizeImage(im1_vector, w, h);
	// Update image width and height according to the resizing
	w = floor(w / 4);
	h = floor(h / 4);

	// Grayscale the images
	std::vector<unsigned char> im0_grey(w*h), im1_grey(w * h);
	printf("Grayscaling im0.png and im1.png on different threads\n");
	// Timing
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);
	// Start grayscaling on different threads
	std::thread grey_thread1(GrayScaleImage, std::ref(im0_grey), std::ref(im0_vector), w, h);
	std::thread grey_thread2(GrayScaleImage, std::ref(im1_grey), std::ref(im1_vector), w, h);
	// Wait for the threads to finish
	grey_thread1.join(); grey_thread2.join();
	// Stop timer
	std::string action = "Images grayscaled.";
	stopTimer(timer, action);
	// Save the grayscaling result
	if (WriteImage("imgs/im0_gray.png", im0_grey, w, h, LCT_GREY, 8)) return 1;
	if (WriteImage("imgs/im1_gray.png", im1_grey, w, h, LCT_GREY, 8)) return 1;

	// Free RGBA images from memory
	FreeImageVector(im0_vector);
	FreeImageVector(im1_vector);

	// Must create Disparity maps for both, since they are required for the Cross Check
	std::vector<unsigned char> dmap0(w * h), dmap1(w * h);
	printf("Creating a ZNCC disparity map. Left img=im0.png, Right img=im1.png\n");
	dmap0 = CalcZNCC(im0_grey, im1_grey, w, h, WINDOW_Y, WINDOW_X, MIN_DISPARITY, MAX_DISPARITY);
	printf("Creating a ZNCC disparity map. Left img=im1.png, Right img=im0.png\n");
	dmap1 = CalcZNCC(im1_grey, im0_grey, w, h, WINDOW_Y, WINDOW_X, -MAX_DISPARITY, MIN_DISPARITY);
	// Save resulting images
	if (WriteImage("imgs/im0_disparity.png", dmap0, w, h, LCT_GREY, 8)) return 1;
	if (WriteImage("imgs/im1_disparity.png", dmap1, w, h, LCT_GREY, 8)) return 1;

	// Free grayscale images from memory
	FreeImageVector(im0_grey);
	FreeImageVector(im1_grey);

	// Perform the Cross Check
	printf("Performing Cross Check between im0 and im1\n");
	std::vector<unsigned char> cross(w * h);
	cross = CrossCheck(dmap0, dmap1, w, h, THRESHOLD);
	// Save resulting image
	if (WriteImage("imgs/cross_check.png", cross, w, h, LCT_GREY, 8)) return 1;

	// Occlusion fill
	printf("Performing Occlusion Fill by finding nearest non-zero neighbors\n");
	std::vector<unsigned char> fill(w * h);
	fill = OcclusionFill(cross, w, h);
	if (fill.size() == 0) return 1;
	// Save resulting image
	if (WriteImage("imgs/occlusion_fill.png", fill, w, h, LCT_GREY, 8)) return 1;

	// Normalize the images
	printf("Normalizing The disparity images\n");
	// Timing
	QueryPerformanceFrequency(&timer.freq);
	QueryPerformanceCounter(&timer.start);
	dmap0 = NormalizeImage(dmap0, w, h);
	dmap1 = NormalizeImage(dmap1, w, h);
	cross = NormalizeImage(cross, w, h);
	fill = NormalizeImage(fill, w, h);
	// Stop timer
	action = "Images normalized.";
	stopTimer(timer, action);

	// Save the normalized images
	if (WriteImage("imgs/im0_disparity_norm.png", dmap0, w, h, LCT_GREY, 8)) return 1;
	if (WriteImage("imgs/im1_disparity_norm.png", dmap1, w, h, LCT_GREY, 8)) return 1;
	if (WriteImage("imgs/cross_check_norm.png", cross, w, h, LCT_GREY, 8)) return 1;
	if (WriteImage("imgs/occlusion_fill_norm.png", fill, w, h, LCT_GREY, 8)) return 1;

	// Free Occlusion Fill and Cross check
	FreeImageVector(dmap0);
	FreeImageVector(dmap1);
	FreeImageVector(cross);
	FreeImageVector(fill);

	printf("Done!\n\n");
	return 0;
}