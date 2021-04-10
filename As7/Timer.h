#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <Windows.h>

/*
* \brief This struct contians all the variables needed for timining executions with QueryPerformanceCounter
*/
typedef struct {
	LARGE_INTEGER freq;
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	LARGE_INTEGER elapsed;
} timer_struct;

/*
* \brief Start execution time counting in the given timee_struct
* \param timer timer_struct to use
*/
void StartTimer(timer_struct* timer);

/*
* \brief Stops the given timer and outputs a custon message which includes the execution time
* \param timer timer_struct Currently counting elapsed time
* \param action Text describing what was timed
*/
void StopTimer(timer_struct* timer, const char* action);


#endif