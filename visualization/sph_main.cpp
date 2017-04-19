#include "sph_header.h"
#include "sph_system.h"
#include "sph_timer.h"
#include "sph_common.h"
#include "sph_visual.h"

Timer *sph_timer;
SPHSystem *sph;
float input_cutoff_ratio;

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

  input_cutoff_ratio = read_float( argc, argv, "-cr", 0.45f );

	// visualization by OpenGL
	visualization(argc, argv);


	free(sph);

	return 0;
}
