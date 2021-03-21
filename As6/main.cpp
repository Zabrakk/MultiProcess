#include <vector>
#include <math.h>

#include "ImageFunctions.h"

#define WINDOW_Y 13 
#define WINDOW_X 11 
#define MIN_DISPARITY 0
#define MAX_DISPARITY 65 // Scaled down. 260/4 as stated in the Assignment
#define THRESHOLD 3

int main() {
	// Set image dimensions
	unsigned w = 2940;
	unsigned h = 2016;
	// Initialize original image vector
	std::vector<unsigned char> im0, im1;

	// Read the images into memory
	ReadImage(im0, "im0.png", w, h);
	ReadImage(im1, "im1.png", w, h);
	printf("\n");

	// Resize the images
	printf("Resizing im0.png\n");
	std::vector<unsigned char> im0_resized = ResizeImage(im0, w, h);
	printf("Resizing im1.png\n");
	std::vector<unsigned char> im1_resized = ResizeImage(im1, w, h);
	printf("\n");

	// Update image dimentions to match the resizing
	w = floor(w / 4);
	h = floor(h / 4);

	// Grayscale the images
	printf("Grayscaling im0.png\n");
	std::vector<unsigned char> im0_gray = GrayScaleImage(im0_resized, w, h);
	printf("Grayscaling im1.png\n");
	std::vector<unsigned char> im1_gray = GrayScaleImage(im1_resized, w, h);
	printf("\n");

	// Free the resized and original images
	FreeImageVector(im0);
	FreeImageVector(im1);
	FreeImageVector(im0_resized);
	FreeImageVector(im1_resized);

	// Calculate ZNCC
	printf("Calculating ZNCC, left=im0, right=im1\n");
	std::vector<unsigned char> im0_zncc = CalcZNCC(im0_gray, im1_gray, w, h, WINDOW_Y, WINDOW_X, MIN_DISPARITY, MAX_DISPARITY);
	printf("\n");

	// Save result
	WriteImage(im0_zncc, "imgs/result.png", w, h, LCT_GREY, 8);

	printf("\nDone!\n");
	return 0;
}