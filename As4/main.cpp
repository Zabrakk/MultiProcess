#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include "lodepng.h"
#include "ImageFunctions.h"

int main() {
	// Iamge width and height
	unsigned w = 207;
	unsigned h = 207;


	std::vector<unsigned char> img;
	img = ReadImage("dank.png", w, h);
	if (img.size() < 1) return 1;

	std::vector<unsigned char> grayscaled(w * h);
	grayscaled = GrayScaleImage(img, w, h);

	if (!WriteImage("result.png", grayscaled, w, h, LCT_GREY)) return 1;

	return 0;
}