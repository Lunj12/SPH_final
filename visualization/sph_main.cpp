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
#include "sph_common.h"

Timer *sph_timer;
char *window_title;

float window_width;
float window_height;

SPHSystem *sph;

void  init_sph_system(float input_cutoff_ratio)
{
	sph=new SPHSystem(input_cutoff_ratio);
	sph->init_system();
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
	if( find_option( argc, argv, "-h" ) >= 0 )
	    {
	        printf( "Options:\n" );
	        printf( "-h to see this help\n" );
	        printf( "-cr <float> to set the ratio of cutoff to kernel range\n" );
	        printf( "-o <filename> to specify the output file name\n" );
	        printf( "-s <filename> to specify a summary file name\n" );
	        printf( "-no turns off all correctness checks and particle output\n");
	        return 0;
	    }

	    float input_cutoff_ratio = read_float( argc, argv, "-cr", 0.45f );
			//
	    // char *savename = read_string( argc, argv, "-o", NULL );
	    // char *sumname = read_string( argc, argv, "-s", NULL );
			//
	    // FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
	    // FILE *fsum = sumname ? fopen ( sumname, "a" ) : NULL;

	glutInit(&argc, argv);

	init_sph_system(input_cutoff_ratio);

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
