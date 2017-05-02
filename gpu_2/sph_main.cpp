#include "sph_header.h"
#include "sph_system.h"
#include "sph_common.h"

SPHSystem *sph;
int NSTEPS;
int SAVEFREQ = 1;

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
					printf( "-Nt <int> total time steps\n");
					printf( "-o <filename> to specify the output file name\n" );
	        printf( "-s <filename> to specify a summary file name\n" );
					return 0;
	    }

	char *savename = read_string( argc, argv, "-o", NULL );
  FILE *fsave = savename ? fopen( savename, "w" ) : NULL;

	// input_cutoff_ratio = read_float( argc, argv, "-cr", 0.45f );
	NSTEPS = read_int( argc, argv, "-Nt", 100 );

	// non-visualization, save the data
	// sph=new SPHSystem(input_cutoff_ratio);
	sph=new SPHSystem();
	sph->init_system();
	sph->init_boundary();

	// run
	sph->sys_running=1-sph->sys_running;
	// record time
	double simulation_time = read_timer( );
	for(int step=0;step < 1; step++)
	{
		sph->animation();
		printf("posx=%f\n", sph->hMem[100].pos.x);
		// save data at proper time
		if( fsave && (step%SAVEFREQ) == 0 )
		{
		 	fprintf(fsave,"Time Step = %d\n",step);
		 	save( fsave, sph->hParam->num_particle, sph );
		}
	}

	simulation_time = read_timer( ) - simulation_time;
	printf( "n = %d, simulation time = %g seconds\n", sph->hParam->num_particle, simulation_time);

	if( fsave )
		fclose( fsave );

	free(sph);

	return 0;
}
