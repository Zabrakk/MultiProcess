#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <Windows.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include "lodepng.h"
#include "OpenCLFunctions.h"

/* The code must do the following
* 1. Load image "im0.png" with lodepng.cpp (DONE)
* 2. Convert  to gray scale with a custom function
* 3. Apply 5x5 moving filter on the gray scaled image with a custom function
* 4. Save final image, use lodepng.cpp
* 6. Display platform/device information
* 7. Display intermediate results for steps 1-4
* 8. Use profiling to get execution times for each of the tasks 1-4, display result
* 9. Measure memory usage (OPTIONAL)
*/

/* Sources:
* HelloWorld Report.pdf from the course
* OpenCL Programming Guide provided on the course
* "Acquiring high-resolution time stamps" - https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
* LodePNG Decode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
* LodePNG Encode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_encode.cpp
*/

int main() {
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue cmd_q = NULL;
	cl_event event = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_int err_num = NULL;
	cl_uint num_of_platforms;
	cl_uint num_of_devices;

	LARGE_INTEGER start, end, elapsed;
	LARGE_INTEGER freq;

	// Obtain first OpenCL platform
	err_num = clGetPlatformIDs(1, &platform_id, &num_of_platforms);
	if (!errorCheck(err_num)) return 1;
	// Print number of platforms
	printf("Number of platforms: %d\n", num_of_platforms);
	// Obtain first device from the selected platform with type GPU
	err_num = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices);
	// Print number of devices on the selected platform
	printf("Number of devices on the selected platform: %d\n\n", num_of_devices);
	if (!errorCheck(err_num)) return 1;
	// Create the context
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Print info about the selected device
	printDeviceInfo(device_id);
	// Create command queue with profiling enabled, so Kernel exewcution time can be obtained
	cmd_q = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err_num);

	/**********************************************/
	/*				  LOAD IMAGE                  */ 
	/**********************************************/

	unsigned w = 2988; // Image width
	unsigned h = 2008; // Image height
	char* file_name = "im0.png"; // Name of the image file
	std::vector<unsigned char> img; // This vector will have w * h * 4 elements representing R,G,B,A

	printf("Loading %s to memory\n", file_name);
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	
	// Loads and decodes. 4 bytes per pixel, ordered R,G,B,A,R,G,B,A ...
	if (lodepng::decode(img, w, h, (const char*)file_name) != 0) {
		printf("Failed to load %s\n\n", file_name);
		getchar();
		return 1;
	}
	printf("\nImage vector size: %d\n", img.size());

	// Stop measuring, image loaded
	QueryPerformanceCounter(&end);
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= freq.QuadPart;
	printf("%s loaded to memory. Took %ld microseconds\n\n", file_name, elapsed);

	int c = 0;
	for (int i = 0; i < img.size(); i++) {
		if (c == 0) {
			img[i] = (unsigned char)((int)img[i] * 0.114); // 0.3
			c += 1;
		}
		else if (c == 1) {
			img[i] = (unsigned char)((int)img[i] * 0.3); // 0.59
			c += 1;
		}
		else if (c == 2) {
			img[i] = (unsigned char)((int)img[i] * 0.59); // 0.114
			c += 1;
		}
		else if (c == 3) {
			c = 0;
		}
	
	}

	/**********************************************/
	/*				  SAVE IMAGE                  */
	/**********************************************/

	
	printf("Saving the resulting image as result.png\n");
	// Start timer
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	// Encode as PNG, and save to "result.png"
	lodepng::encode("result.png", img, w, h);
	// Stop timer
	QueryPerformanceCounter(&end);
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= freq.QuadPart;
	printf("Image saved. Took %ld microseconds\n\n", elapsed);
	

	/**********************************************/
	/*				  FINALIZATION                */
	/**********************************************/

	//free(img);
	printf("Done. Enter a char to exit!\n");
	getchar();
	return 0;
}