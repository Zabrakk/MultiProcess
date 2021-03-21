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
	printf("Calculating ZNCC, left=im1, right=im0\n");
	std::vector<unsigned char> im1_zncc = CalcZNCC(im1_gray, im0_gray, w, h, WINDOW_Y, WINDOW_X, -MAX_DISPARITY, MIN_DISPARITY);
	printf("\n");

	// Cross Check
	printf("Performing the cross check\n");
	std::vector<unsigned char> cross = CrossCheck(im0_zncc, im1_zncc, w, h, THRESHOLD);
	printf("\n");

	// Occlusion Fill
	printf("Performing the occlusion fill\n");
	std::vector<unsigned char> fill = OcclusionFill(cross, w, h);
	printf("\n");

	// Image Normalization
	printf("Normalizing the images\n");
	im0_zncc = NormalizeImage(im0_zncc, w, h);
	im1_zncc = NormalizeImage(im1_zncc, w, h);
	cross = NormalizeImage(cross, w, h);
	fill = NormalizeImage(fill, w, h);
	printf("\n");

	// Save results
	WriteImage(im0_zncc, "imgs/im0_zncc_norm.png", w, h, LCT_GREY, 8);
	WriteImage(im1_zncc, "imgs/im1_zncc_norm.png", w, h, LCT_GREY, 8);
	WriteImage(cross, "imgs/cross_check_norm.png", w, h, LCT_GREY, 8);
	WriteImage(fill, "imgs/occlusion_fill_norm.png", w, h, LCT_GREY, 8);

	// And we are done
	printf("\nDone!\n");
	return 0;
}