#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

#include "sph_header.h"
#include "sph_system.h"
#include "sph_timer.h"


Timer *sph_timer;
char *window_title;

float window_width;
float window_height;

SPHSystem *sph;

void  init_sph_system()
{
	sph=new SPHSystem();
	sph->init_system();
  sph->init_boundary();
	sph->init_body();

	sph_timer=new Timer();
	window_title=(char *)malloc(sizeof(char)*50);

	window_width=600;
	window_height=window_width/sph->world_size.x*sph->world_size.y;
}

void init()
{
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluOrtho2D(0.0, sph->world_size.x, 0.0, sph->world_size.y);
}

void display_particle()
{
	glColor3f(0.2f, 0.2f, 1.0f);
	glPointSize(5.0f);

	/*glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, sph->hPoints);
	glDrawArrays(GL_POINTS, 0, sph->hParam->num_particle);*/

	for(uint i=0; i<sph->num_particle; i++)
	{
		if(sph->mem[i].surf_norm > sph->surf_norm)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else
		{
			glColor3f(0.2f, 0.2f, 1.0f);
		}

		glBegin(GL_POINTS);
		glVertex2f(sph->mem[i].pos.x, sph->mem[i].pos.y);
		glEnd();
	}
  // printf("NUM_PARTICLE_BC=%ld\n", sph->num_particle_bc);
	for(uint i=0; i<sph->num_particle_bc; i++)
  {
		glColor3f(0.2f, 0.8f, 1.0f);
		glBegin(GL_POINTS);
		glVertex2f(sph->BC[i].pos.x, sph->BC[i].pos.y);
		glEnd();
	}

	for(uint i=0; i<sph->num_particle_body; i++)
  {
		glColor3f(1.0f, 0.8f, 1.0f);
		glBegin(GL_POINTS);
		glVertex2f(sph->BODY[i].pos.x, sph->BODY[i].pos.y);
		glEnd();
	}
}

void display_func()
{
	glViewport(0, 0, window_width, window_height);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0f, 0.0f, 1.0f);

	sph->animation();
	display_particle();

	glutSwapBuffers();

	sph_timer->update();
	memset(window_title, 0, 50);
	sprintf(window_title, "SPH System 2D. FPS: %0.2f", sph_timer->get_fps());
	glutSetWindowTitle(window_title);
}

void idle_func()
{
	glutPostRedisplay();
}

void reshape_func(int width, int height)
{
	window_width=width;
	window_height=height;
	glutReshapeWindow(window_width, window_height);
}

void process_keyboard(unsigned char key, int x, int y)
{
	if(key == ' ')
	{
		sph->sys_running=1-sph->sys_running;
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	init_sph_system();

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("SPH System 2D");

	init();

  glutDisplayFunc(display_func);
	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutKeyboardFunc(process_keyboard);
  glutMainLoop();

	free(sph);

	return 0;
}
