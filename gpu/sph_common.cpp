#include <assert.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>
#include "sph_common.h"
#include "sph_system.h"

//
//  timer
//
double read_timer( )
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

//
//  I/O routines
//
void save( FILE *f, int n, SPHSystem *sph)
{
    static bool first = true;
    if( first )
    {
        fprintf( f, "n = %d size = %g\n", sph->hParam->num_particle, sph->hParam->world_size.x);
        first = false;
    }
    for( int i = 0; i < n; i++ )
        fprintf( f, "%g %g\n", sph->hMem[i].pos.x, sph->hMem[i].pos.y);
}

//
//  command line option processing
//
int find_option( int argc, char **argv, const char *option )
{
    for( int i = 1; i < argc; i++ )
        if( strcmp( argv[i], option ) == 0 )
            return i;
    return -1;
}

int read_int( int argc, char **argv, const char *option, int default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atoi( argv[iplace+1] );
    return default_value;
}

float read_float( int argc, char **argv, const char *option, float default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atof( argv[iplace+1] );
    return default_value;
}


char *read_string( int argc, char **argv, const char *option, char *default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return argv[iplace+1];
    return default_value;
}
