#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED
// Standard include guard

#include <stdio.h>
#include <string.h>
#include <CL/cl.h> // OpenCL

/**
* \brief Selects the first available OpenCL GPU platform and get its context
* \return cl_context if platform found; NULL otherwise
*/
cl_context create_GPU_context();

/**
* \brief CURRENTLY NOT WORKING Select the first available OpenCL CPU platform and get its context
* \return cl_context if platform found; NULL otherwise
*/
cl_context create_CPU_context();

/**
* \brief Select the first CPU or GPU platform and get its context
* \param type Either "CPU" or "GPU". Decides which platform to obtain
* \return cl_context if platform found; NULL otherwise
*/
cl_context create_platform_context(char type[]);

// End include guard
#endif