#ifndef ERRORCHECK_H_INCLUDED
#define ERRORCHECK_H_INCLUDED
// Standard include guard

#include <stdio.h>
#include <CL/cl.h> // OpenCL

/**
* \brief Check if an OpenCL error occured. Prints info about the possible error
* \param errNum cl_int which should be checked for possible errors
* \return bool true if no error; false otherwise 
*/
bool error_check(cl_int errNum);


// End include guard
#endif