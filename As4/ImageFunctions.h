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
* \brief Uses lodepng to read the given image into memory using the RGBA format. The image vector will be R,G,B,A,R,G,B,A...
* \param filename Name of the image file
* \param w Width of the image
* \param h Height of the image
* \return Unsigned char vector containing the image or an empry vector if failed
*/
std::vector<unsigned char> ReadImage(std::string filename, unsigned int w, unsigned int h);

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
* \brief Downscales the given image by 4
* \param img Image to downscale
* \return Downscaled image or NULL if failed
*/
std::vector<unsigned char> ResizeImage(std::vector<unsigned char> img);

/*
* \brief Grayscales the given image RGBA image
* \param img Image to downscale
* \param w Width of the image
* \param h Height of the image
* \return Grayscaled image
*/
std::vector<unsigned char> GrayScaleImage(std::vector<unsigned char> img, unsigned int w, unsigned int h);


#endif