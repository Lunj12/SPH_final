#include "sph_system.h"
#include "sph_timer.h"
#include "sph_common.h"
#include "sph_visual.h"

Timer *sph_timer;
SPHSystem *sph;
float input_world_size;
float input_cutoff_ratio;
int vis_switch;
int NSTEPS;
int SAVEFREQ = 1;

int main(int argc, char **argv)
{
	if( find_option( argc, argv, "-h" ) >= 0 )
	    {
	        printf( "Options:\n" );
	        printf( "-h to see this help\n" );
					printf( "-w <float> to set the world size\n" );
	        printf( "-cr <float> to set the ratio of cutoff to kernel range\n" );
					printf( "-v <string> turns on/off all visualization\n");
					printf( "-Nt <int> total time steps\n");
					printf( "-o <filename> to specify the output file name\n" );
	        printf( "-s <filename> to specify a summary file name\n" );
					printf( " Space to pause/resume simulation\n" );
					return 0;
	    }

	char *savename = read_string( argc, argv, "-o", NULL );
  FILE *fsave = savename ? fopen( savename, "w" ) : NULL;

	input_cutoff_ratio = read_float( argc, argv, "-cr", 0.45f );
	input_world_size = read_float( argc, argv, "-w", 1.28f );
	vis_switch = read_int( argc, argv, "-v", 1 );
	NSTEPS = read_int( argc, argv, "-Nt", 100 );

	// visualization by OpenGL
	if (vis_switch == 1)
	{
	visualization(argc, argv);
	}

	// non-visualization, save the data
	if (vis_switch == 0) {
		sph=new SPHSystem(input_world_size, input_cutoff_ratio);
		sph->init_system();
		sph->init_boundary();
		sph->init_body();

		// run
		sph->sys_running=1-sph->sys_running;
		// record time
		double simulation_time = read_timer( );
		for(int step=0;step < NSTEPS; step++)
		{
			sph->animation();
			printf("%f\n", sph->mem[200].pos.x);
			// save data at proper time
			if( fsave && (step%SAVEFREQ) == 0 )
			{
			 	fprintf(fsave,"Time Step = %d\n",step);
			 	save( fsave, sph->num_particle, sph );
			}
		}

		simulation_time = read_timer( ) - simulation_time;
		printf( "n = %d, simulation time = %g seconds\n", sph->num_particle, simulation_time);
	}

	if( fsave )
		fclose( fsave );

	free(sph);

	return 0;
}
