#ifndef __SPHCOMMON_H__
#define __SPHCOMMON_H__

inline int min( int a, int b ) { return a < b ? a : b; }
inline int max( int a, int b ) { return a > b ? a : b; }

//
//  saving parameters
//
const int NSTEPS = 1000;
const int SAVEFREQ = 10;

//
//  timing routines
//
double read_timer( );

//
//  I/O routines
//
FILE *open_save( char *filename, int n );
// void save( FILE *f, int n, particle_t *p );

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
float read_float( int argc, char **argv, const char *option, float default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

#endif
