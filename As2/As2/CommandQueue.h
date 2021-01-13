#ifndef COMMANDQUEUE_H_INCLUDED
#define COMMANDQUEUE_H_INCLUDED
// Standard include guard

/**
* The Command Queue is used to queue the kernel for execution and to read its results
*/

#include <stdio.h>
#include <CL/cl.h>

/**
* \brief Creates a OpenCL command queue for the given device
* \param context Context of the target device
* \param device ID of the device
*/
cl_command_queue create_command_queue(cl_context context, cl_device_id* device);

// End include guard
#endif