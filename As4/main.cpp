#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include "lodepng.h"
#include "ImageFunctions.h"

#define MIN_DISPARITY 0
#define MAX_DISPARITY 61
#define THRESHOLD 3

int main() {
	// Image width and height. Both input images are the same size
	unsigned w = 2940;
	unsigned h = 2016;

	// Original images
	std::vector<unsigned char> im0, im1;
	// Intermediate results
	std::vector<unsigned char> im0_grey, im1_grey, dmap0, dmap1, cross, fill;
	// Final normalized results
	std::vector<unsigned char> n_dmap0, n_dmap1, n_cross, n_fill;
	
	// Read the images
	im0 = ReadImage("im0.png", w, h, LCT_RGBA);
	if (im0.size() < 1) return 1;
	im1 = ReadImage("im1.png", w, h, LCT_RGBA);
	if (im1.size() < 1) return 1;

	// Resize both images
	printf("Donwscaling im0.png by four.");
	im0 = ResizeImage(im0, w, h);
	printf("Donwscaling im1.png by four.");
	im1 = ResizeImage(im1, w, h);
	
	// Update image width and height according to the resizing
	w = floor(w / 4);
	h = floor(h / 4);
	
	// Grayscale the images
	printf("Grayscaling im0.png\n");
	im0_grey = GrayScaleImage(im0, w, h);
	printf("Grayscaling im1.png\n");
	im1_grey = GrayScaleImage(im1, w, h);
	// Save the resulting images
	if (!WriteImage("im0_grey.png", im0_grey, w, h, LCT_GREY)) return 1;
	if (!WriteImage("im1_grey.png", im1_grey, w, h, LCT_GREY)) return 1;

	// Free RGBA image from memory
	//printf("Freeing memory allocated to the RGBA images\n\n");
	FreeImageVector(im0);
	FreeImageVector(im1);

	// im0 = Left image, im1 = Right image
	// Must create Disparity maps for both, since they are required for the Cross Check
	printf("Creating a ZNCC disparity map. Left img=im0.png, Right img=im1.png\n");
	dmap0 = CalcZNCC(im0_grey, im1_grey, w, h, MIN_DISPARITY, MAX_DISPARITY);
	printf("Creating a ZNCC disparity map. Left img=im1.png, Right img=im0.png\n");
	dmap1 = CalcZNCC(im1_grey, im0_grey, w, h, -MAX_DISPARITY, MIN_DISPARITY);
	if (!WriteImage("im0_disparity.png", dmap0, w, h, LCT_GREY)) return 1;
	if (!WriteImage("im1_disparity.png", dmap1, w, h, LCT_GREY)) return 1;

	// Free grayscale images from memory
	FreeImageVector(im0_grey);
	FreeImageVector(im1_grey);

	// Cross Check
	printf("Performing Cross Check between im0 and im1\n");
	cross = CrossCheck(dmap0, dmap1, w, h, THRESHOLD);
	if (!WriteImage("cross_check.png", cross, w, h, LCT_GREY)) return 1;
	
	// Occlusion fill
	printf("Performing Occlusion Fill by finding nearest non-zero neighbors\n");
	fill = OcclusionFill(cross, w, h);
	if (fill.size() == 0) return 1;
	if (!WriteImage("occlusion_fill.png", fill, w, h, LCT_GREY)) return 1;
	
	// Normalize and save
	printf("Normalizing im0.png disparity map\n");
	n_dmap0 = NormalizeImage(dmap0, w, h);
	if (!WriteImage("im0_normalized.png", n_dmap0, w, h, LCT_GREY)) return 1;
	printf("Normalizing im1.png disparity map\n");
	n_dmap1 = NormalizeImage(dmap1, w, h);
	if (!WriteImage("im1_normalized.png", n_dmap1, w, h, LCT_GREY)) return 1;
	printf("Normalizing Cross Check\n");
	n_cross = NormalizeImage(cross, w, h);
	if (!WriteImage("cross_check_normalized.png", n_cross, w, h, LCT_GREY)) return 1;
	printf("Normalizing Occulsion Fill\n");
	n_fill = NormalizeImage(fill, w, h);
	if (!WriteImage("occlusion_fill_normalized.png", n_fill, w, h, LCT_GREY)) return 1;
	
	// Finalization
	FreeImageVector(dmap0);
	FreeImageVector(dmap1);
	FreeImageVector(cross);
	FreeImageVector(fill);

	FreeImageVector(n_dmap0);
	FreeImageVector(n_dmap1);
	FreeImageVector(n_cross);
	FreeImageVector(n_fill);

	return 0;
}