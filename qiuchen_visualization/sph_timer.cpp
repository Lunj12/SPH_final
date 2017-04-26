#include "sph_timer.h"
#include <sys/time.h>
#include <cstddef>

Timer::Timer()
{
	frames=0;
	update_time=1000;
	last_time=0;
	FPS=0;
}

void Timer::update()
{
	frames++;

	if(GetTickCount()-last_time > update_time)
	{
		FPS=((double)frames/(double)(GetTickCount()-last_time))*1000.0;
		last_time=GetTickCount();
		frames=0;
	}
}

double Timer::get_fps()
{
	return FPS;
}

unsigned GetTickCount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
	return 0;

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
