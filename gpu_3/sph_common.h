#ifndef __SPHCOMMON_H__
#define __SPHCOMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "sph_system.h"
//
// #define PI 3.141592f
// #define INF 1E-12f
// #define BOUNDARY 0.001f

//
//  timing routines
//
double read_timer( );

//
//  I/O routines
//
FILE *open_save( char *filename, int n );
void save( FILE *f, int n, SPHSystem *sph);

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
float read_float( int argc, char **argv, const char *option, float default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

#endif
