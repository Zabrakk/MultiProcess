#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include "lodepng.h"
#include "ImageFunctions.h"

int main() {
	// Image width and height. Both input images are the same size
	unsigned w = 2940;
	unsigned h = 2016;
	// Size after downscaling
	unsigned new_w;
	unsigned new_h;

	// Allocate vectors for the input images
	std::vector<unsigned char> im0;
	std::vector<unsigned char> im1;
	// Read the images
	im0 = ReadImage("im0.png", w, h);
	if (im0.size() < 1) return 1;
	im1 = ReadImage("im1.png", w, h);
	if (im1.size() < 1) return 1;
	// Resize both images
	im0 = ResizeImage(im0, w, h, new_w, new_h);
	im1 = ResizeImage(im1, w, h, new_w, new_h);
	// Allocate vectors for grayscaled images
	std::vector<unsigned char> im0_gray(new_w * new_h);
	std::vector<unsigned char> im1_gray(new_w * new_h);
	// Grayscale the images
	im0_gray = GrayScaleImage(im0, new_w, new_h);
	im1_gray = GrayScaleImage(im1, new_w, new_h);
	// Save the resulting images
	if (!WriteImage("im0_grey.png", im0_gray, new_w, new_h, LCT_GREY)) return 1;
	if (!WriteImage("im1_grey.png", im1_gray, new_w, new_h, LCT_GREY)) return 1;


	return 0;
}