#ifndef __SPHTIMER_H__
#define __SPHTIMER_H__

class Timer
{
private:
	int frames;
	int update_time;
	int last_time;
	double FPS;

public:
	Timer();
	void update();
	double get_fps();
};

unsigned GetTickCount();

#endif
