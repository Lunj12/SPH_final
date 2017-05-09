#include "sph_system.h"
#include "sph_common.h"

SPHSystem *sph;
float input_cutoff_ratio;
float input_world_size;
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
					printf( "-Nt <int> total time steps\n");
					printf( "-o <filename> to specify the output file name\n" );
	        printf( "-s <filename> to specify a summary file name\n" );
					return 0;
	    }

	char *savename = read_string( argc, argv, "-o", NULL );
  FILE *fsave = savename ? fopen( savename, "w" ) : NULL;

	input_cutoff_ratio = read_float( argc, argv, "-cr", 0.45f );
	input_world_size = read_float( argc, argv, "-w", 1.28f );
	NSTEPS = read_int( argc, argv, "-Nt", 100 );

	// non-visualization, save the data
	sph=new SPHSystem(input_world_size, input_cutoff_ratio);

	// initialize particles boundaries and bodies
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

		// save data at proper time
		if( fsave && (step%SAVEFREQ) == 0 )
		{
		 	fprintf(fsave,"Time Step = %d\n",step);
		 	save( fsave, sph->num_particle, sph );
		}
	}

	simulation_time = read_timer( ) - simulation_time;
	printf( "N = %d, simulation time = %g seconds\n", sph->num_particle + sph->num_particle_bc + sph->num_particle_body, simulation_time);

	if( fsave )
		fclose( fsave );

	free(sph);

	return 0;
}
