#ifndef __SPHVISUAL_H__
#define __SPHVISUAL_H__

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

#include "sph_system.h"

void  init_sph_system(float input_cutoff_ratio);
void init();
void display_particle();
void display_func();
void idle_func();
void reshape_func(int width, int height);
void process_keyboard(unsigned char key, int x, int y);
void visualization(int argc, char **argv);

#endif
