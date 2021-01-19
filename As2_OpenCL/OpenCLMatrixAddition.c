#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include "MatrixFunctions.h"
#include "OpenCLFunctions.h"

#define SIZE 100 // Define size of matrices (size x size)
#define KERNEL_FINE_NAME "add_matrix.cl" // Kernel file name
#define KERNEL_NAME "add_matrix" // Kernel Function name


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

	cl_mem m1_mem = NULL;
	cl_mem m2_mem = NULL;
	cl_mem result_mem = NULL;

	// Load Kernel source into memory
	kernel_source src = loadKernel(KERNEL_FINE_NAME);
	if (src.ok == 0) return 1;

	// Obtain first OpenCL platform
	err_num = clGetPlatformIDs(1, &platform_id, &num_of_platforms);
	if (!errorCheck(err_num)) return 1;
	// Obtain first device from the selected platform with type GPU
	err_num = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices);
	if (!errorCheck(err_num)) return 1;
	// Create the context
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Print info about the selected device
	printDeviceInfo(device_id);
	// Create command queue with profiling enabled, so Kernel exewcution time can be obtained
	cmd_q = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err_num);

	// Create the matrices
	int* m1 = createMatrix(SIZE);
	int* m2 = createMatrix(SIZE);
	int* result = createMatrix(SIZE);
	// Create memory buffers for the matrices
	m1_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, SIZE * SIZE * sizeof(int), m1, &err_num);
	m2_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, SIZE * SIZE * sizeof(int), m2, &err_num);
	result_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE * SIZE * sizeof(int), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Create the program with Kernel source
	program = clCreateProgramWithSource(context, 1, (const char**)&src.source_str, (const size_t*)&src.source_size, &err_num);
	// Build the program
	err_num = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	//Print logs if build failed
	if (err_num == CL_BUILD_PROGRAM_FAILURE) {
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		// Allocate memory for the log
		char* log = (char*)malloc(log_size);
		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		// Print the log
		printf("%s\n", log);
	}
	// Create the actual Kernel
	kernel = clCreateKernel(program, KERNEL_NAME, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Set the matrices' as arguments for the Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &m1_mem);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &m2_mem);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &result_mem);
	if (!errorCheck(err_num)) return 1;

	// Set global and loval work sizes
	size_t global_work_size[1] = { SIZE * SIZE };
	size_t local_work_size[1] = { SIZE }; // Each row is it's own local work group
	// Execute the Kernel, give event so execution time can be obtained
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	cl_ulong start = 0, end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
	double milliseconds = (cl_double)(end - start) * (cl_double)(1e-06);
	// Get the resulting matrix
	err_num = clEnqueueReadBuffer(cmd_q, result_mem, CL_TRUE, 0, SIZE * SIZE * sizeof(int), result, 0, NULL, NULL);
	if (!errorCheck(err_num)) return 1;

	// Output matrix and execution time
	//printMatrix(result, SIZE);
	printf("Kernel execution took %f milliseconds\n", milliseconds);

	// Finalize by freeing memory
	err_num = clFlush(cmd_q);
	err_num = clFinish(cmd_q);
	err_num = clReleaseKernel(kernel);
	err_num = clReleaseProgram(program);
	err_num = clReleaseMemObject(m1_mem);
	err_num = clReleaseMemObject(m2_mem);
	err_num = clReleaseMemObject(result_mem);
	err_num = clReleaseCommandQueue(cmd_q);
	err_num = clReleaseContext(context);

	free(src.source_str);
	freeMatrix(m1);
	freeMatrix(m2);
	freeMatrix(result);
	getchar();
	return 0;
}