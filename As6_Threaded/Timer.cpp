#include <stdio.h>
#include <Windows.h>
#include "Timer.h"

void StartTimer(timer_struct* timer) {
	QueryPerformanceFrequency(&timer->freq);
	QueryPerformanceCounter(&timer->start);
}

void StopTimer(timer_struct* timer, const char* action) {
	QueryPerformanceCounter(&timer->end);
	timer->elapsed.QuadPart = timer->end.QuadPart - timer->start.QuadPart;
	timer->elapsed.QuadPart *= 1000000;
	timer->elapsed.QuadPart /= timer->freq.QuadPart;
	printf("%s. Took %ld microseconds\n", action, timer->elapsed);
}