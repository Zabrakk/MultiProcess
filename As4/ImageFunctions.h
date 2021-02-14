#ifndef IMAGEFUNCTIONS_H_INCLUDED
#define IMAGEFUNCTIONS_H_INCLUDED

#include <vector>
#include <string>

/******************************************************
* THIS IS THE HEADER FOR COMMON IMAGE RELATED FUNCTIONS
******************************************************/

/*
* \brief Struct that holds the Windows API timing parameters
*/
typedef struct {
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	LARGE_INTEGER elapsed;
	LARGE_INTEGER freq;
} timer_struct;

/*
* \brief Stops the Windows API timer and outputs elapsed time
* \param timer timer_struct
* \param action Action that was timed. Will be outputted
* \return Nothing
*/
void stopTimer(timer_struct timer, std::string action);

/*
* \brief Explicitly frees the memory allocated to the given vector
* \param img_vector Image vector to remove from memory
* \return Nothing
*/
void FreeImageVector(std::vector<unsigned char>& img_vector);

/*
* \brief Uses lodepng to read the given image into memory using the specified image format (e.g. LCT_RGBA. LCT_GREY)
* \param filename Name of the image file
* \param w Width of the image
* \param h Height of the image
* \param type LodePNGColorType to use when reading
* \return Unsigned char vector containing the image or an empry vector if failed
*/
std::vector<unsigned char> ReadImage(std::string filename, unsigned int w, unsigned int h, LodePNGColorType type);

/*
* \brief Uses lodepng to save a given image to disk
* \param filename Filename to use
* \param img Image vector
* \param w Width of the image
* \param h Height of the image
* \param type LodePNGColorType to use when saving
* \return True if saved succesfully, false otherwise
*/
bool WriteImage(std::string filename, std::vector<unsigned char> img, unsigned int w, unsigned int h, LodePNGColorType type);

/*
* \brief Downscales the given RGBA image by 4. This is done by dropping pixels
* \param img Image to downscale
* \param w Width of the original image
* \param h Height of the original image
* \return Downscaled image or an error if failed. (0.0) spooky...
*/
std::vector<unsigned char> ResizeImage(std::vector<unsigned char> img, unsigned int w, unsigned int h);

/*
* \brief Grayscales the given image RGBA image
* \param img Image to downscale
* \param w Width of the image
* \param h Height of the image
* \return Grayscaled image
*/
std::vector<unsigned char> GrayScaleImage(std::vector<unsigned char> img, unsigned int w, unsigned int h);

/*
* \brief Calculates Zero-mean Normalized Cross Correlation for two given image
* \param img_left Left image
* \param img_right Right image
* \param w Image width
* \param h Image height
* \param min_disparity Minimum disparity value
* \param max_disparity Maximum disparity value
* \return The result
*/
std::vector<unsigned char> CalcZNCC(std::vector<unsigned char> img_left, std::vector<unsigned char> img_right, unsigned int w, unsigned int h, int min_disparity, int max_disparity);

/*
* \brief Performs a Cross Check between the given left and right image
* \param left Left image
* \param right Right image
* \param w Image width
* \param h Image height
* \param th Threshold value
* \return New image with the result
*/
std::vector<unsigned char> CrossCheck(std::vector<unsigned char> left, std::vector<unsigned char> right, unsigned int w, unsigned int h, unsigned int th);

/*
* \brief Eliminates the zeros created by Cross Checking
* \param cross Result of the Cross Checking
* \param w Image width
* \param h Image height
* \param neighborhood_size Size of the neighborhood
* \return Resulting image
*/
std::vector<unsigned char> OcclusionFill(std::vector<unsigned char> cross, unsigned int w, unsigned int h);

/*
* \brief Finds nearest non-zero pixel from given image's current coordinates neighborhood. Called by OcclusionFill
* \param dmap Image to use
* \param w Image width
* \param h Image height
* \param y Current y coordinate
* \param x Current x coordinate
* \return Non-zero value or -1 if failed to find
*/
int find_nearest(std::vector<unsigned char> dmap, unsigned int w, unsigned int h, int y, int x);

/*
* \brief Normalizes the values of a disparity map making it look nicer
* \param dmap Disparity map
* \param w Image width
* \param h Image height
* \return Normalized image
*/
std::vector<unsigned char> NormalizeImage(std::vector<unsigned char> dmap, unsigned int w, unsigned int h);

#endif