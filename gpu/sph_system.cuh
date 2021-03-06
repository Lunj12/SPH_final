#ifndef __SPHSYSTEM_CU__
#define __SPHSYSTEM_CU__

#include "sph_header.h"
#include "sph_system.h"

#define CUDA_HOST_TO_DEV	1
#define CUDA_DEV_TO_HOST	2
#define CUDA_DEV_TO_DEV		3

void alloc_array(void **dev_ptr, size_t size);
void free_array(void *dev_ptr);
void copy_array(void *ptr_a, void *ptr_b, size_t size, int type);
void copy_buffer(Particle *dMem, float2 *buffer, uint num_particle);
void compute_grid_size(int num_particle, int block_size, int &num_blocks, int &num_threads);
void calc_hash(uint *dHash, uint *dIndex, Particle *dMem, uint num_particle, SysParam* dParam);
void integrate_velocity(Particle *dMem, uint num_particle, SysParam* dParam);
void sort_particles(uint *dHash, uint *dIndex, uint num_particle);
void find_start_end(uint *dStart, uint *dEnd, uint *dHash, uint *dIndex, uint num_particle, uint num_cell);
void compute(Particle *dMem, uint *dHash, uint *dIndex, uint *dStart, uint *dEnd, uint num_particle, uint tot_cell, SysParam* dParam);
void move_body(Particle *dMem, uint num_particle, uint num_particle_bc, SysParam* dParam);
void fix_boundary(Particle *dMem, uint num_particle, uint num_particle_fluid, SysParam* dParam);

#endif
