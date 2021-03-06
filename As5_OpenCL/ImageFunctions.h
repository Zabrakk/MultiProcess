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
* \brienf Convert unsigned char* to a much nicer format, i.e. vector
* \param arr Unsigned char array to convert
* \param size Size of the array (e.g., w*h*4)
* \return A vector
*/
std::vector<unsigned char> charArrToVector(unsigned char* arr, unsigned int size);

/*
* \brief Explicitly frees the memory allocated to the given vector
* \param img_vector Image vector to remove from memory
* \return Nothing
*/
void FreeImageVector(std::vector<unsigned char>& img_vector);

/*
* \brief Uses lodepng to read the given image into memory, Calls lodepng_decode32_file
* \param out The read image is stored here
* \param filename Name of the image file
* \param w Width of the image
* \param h Height of the image
* \return 0 if successful; 1 otherwise
*/
int ReadImage(unsigned char** out, const char* filename, unsigned w, unsigned h);

/*
* \brief Uses lodepng_encode_file to save a given image to disk
* \param filename Filename to use
* \param img Image to save. The image is converted to an unsigned char array before saving
* \param w Width of the image
* \param h Height of the image
* \param type LodePNGColorType to use when saving
* \bitdepth Bit depth to use when saving
* \return 0 if successful; 1 otherwise
*/
int WriteImage(const char* filename, std::vector<unsigned char> img, unsigned int w, unsigned int h, LodePNGColorType type, unsigned bitdepth);


#endif