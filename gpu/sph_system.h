#ifndef __SPHSYSTEM_H__
#define __SPHSYSTEM_H__

#include "sph_header.h"
#include "cutil_math.h"

class SysParam
{
public:
	uint max_particle;
	uint num_particle;

	float kernel;
	float mass;

	float2 world_size;
	float cell_size;
	uint2 grid_size;
	uint tot_cell;

	float2 gravity;
	float wall_damping;
	float rest_density;
	float gas_constant;
	float viscosity;
	float time_step;
	float surf_normal;
	float surf_coe;

	float poly6_value;
	float spiky_value;
	float visco_value;

	float grad_poly6;
	float lplc_poly6;

	float kernel_2;
	float self_dens;
	float self_lplc_color;
};

class Particle
{
public:
	float2 pos;
	float2 vel;
	float2 acc;
	float2 ev;

	float dens;
	float pres;

	float surf_norm;
};

class SPHSystem
{
public:
	Particle *hMem;
	Particle *dMem;
	float2 *hPoints;
	float2 *dPoints;

	uint *dHash;
	uint *dIndex;
	uint *dStart;
	uint *dEnd;

	SysParam *hParam;

	uint sys_running;
	uint vbo;
	cudaGraphicsResource *resource;

public:
	SPHSystem();
	~SPHSystem();
	void animation();
	void init_system();
	void add_particle(float2 pos, float2 vel);
};

#endif
