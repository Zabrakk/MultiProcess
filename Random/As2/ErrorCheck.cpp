#include "ErrorCheck.h"
#include <stdio.h>
#include <CL/cl.h>
/**
* OpenCL error checking
*/

bool error_check(cl_int errNum) {
	if (errNum == CL_DEVICE_NOT_FOUND) {
		printf("ERROR: OpenCL device was not found!\n");
		return false;
	}
	else if (errNum == CL_INVALID_CONTEXT) {
		printf("ERROR: Invalid OpenCL context!\n");
		return false;
	}
	else if (errNum != CL_SUCCESS) {
		printf("ERROR: OpenCL error occured! Error code is %d \n", errNum);
		return false;
	}

	return true;
}