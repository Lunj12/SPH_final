#include "sph_header.h"
#include "sph_system.h"
#include "sph_timer.h"
#include "sph_common.h"

// for window
//#include <GL\glut.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

Timer *sph_timer;
float input_cutoff_ratio;
float input_world_size;
char *window_title;

float window_width;
float window_height;

SPHSystem *sph;

bool init_cuda(void)
{
	int count = 0;
	int i = 0;

	cudaGetDeviceCount(&count);
	if(count == 0)
	{
		fprintf(stderr, "There is no device.\n");
		return false;
	}
	for(i = 0; i < count; i++)
	{
		cudaDeviceProp prop;
		if(cudaGetDeviceProperties(&prop, i) == cudaSuccess)
		{
			if(prop.major >= 1)
			{
				break;
			}
		}
	}
	if(i == count)
	{
		fprintf(stderr, "There is no device supporting CUDA.\n");
		return false;
	}

	cudaSetDevice(i);

	printf("CUDA initialized.\n");
	return true;
}

void  init_sph_system(float input_world_size, float input_cutoff_ratio)
{
	sph=new SPHSystem(input_world_size, input_cutoff_ratio);
	sph->init_system();
  sph->init_boundary();
  sph->init_body();

	sph_timer=new Timer();
	window_title=(char *)malloc(sizeof(char)*50);

	window_width=600;
	window_height=window_width/sph->hParam->world_size.x*sph->hParam->world_size.y;
}

void init()
{
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluOrtho2D(0.0, sph->hParam->world_size.x, 0.0, sph->hParam->world_size.y);
}

void display_particle()
{
	glColor3f(0.2f, 0.2f, 1.0f);
	glPointSize(2.0f);

	//glEnableClientState(GL_VERTEX_ARRAY);
	//glVertexPointer(2, GL_FLOAT, 0, sph->hPoints);
	//glDrawArrays(GL_POINTS, 0, sph->hParam->num_particle);

	for(uint i=0; i<sph->hParam->num_particle_fluid; i++)
	{
		if(sph->hMem[i].surf_norm > sph->hParam->surf_normal)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else
		{
			glColor3f(0.2f, 0.2f, 1.0f);
		}
		glBegin(GL_POINTS);
			glVertex2f(sph->hPoints[i].x, sph->hPoints[i].y);
		glEnd();
	}

	for(uint i=sph->hParam->num_particle_fluid; i<sph->hParam->num_particle_fluid + sph->hParam->num_particle_bc; i++)
	  {
			glColor3f(0.2f, 0.8f, 1.0f);
			glBegin(GL_POINTS);
				glVertex2f(sph->hPoints[i].x, sph->hPoints[i].y);
			glEnd();
		}

	for(uint i=sph->hParam->num_particle_fluid + sph->hParam->num_particle_bc; i<sph->hParam->num_particle_fluid + sph->hParam->num_particle_bc + sph->hParam->num_particle_body; i++)
		{
			glColor3f(1.0f, 0.8f, 1.0f);
			glBegin(GL_POINTS);
				glVertex2f(sph->hPoints[i].x, sph->hPoints[i].y);
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
	init_cuda();
	cudaDeviceProp prop;
    int dev;
	memset( &prop, 0, sizeof( cudaDeviceProp ) );
    prop.major = 1;
    prop.minor = 0;
    cudaChooseDevice( &dev, &prop );

		if( find_option( argc, argv, "-h" ) >= 0 )
	    {
	        printf( "Options:\n" );
	        printf( "-h to see this help\n" );
					printf( "-w <float> to set the world size\n" );
	        printf( "-cr <float> to set the ratio of cutoff to kernel range\n" );
					printf( "Space to pause/reasume simulation\n");
					return 0;
	    }

	input_cutoff_ratio = read_float( argc, argv, "-cr", 0.45f );
	input_world_size = read_float( argc, argv, "-w", 1.28f );

	glutInit(&argc, argv);

	init_sph_system(input_world_size, input_cutoff_ratio);

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
