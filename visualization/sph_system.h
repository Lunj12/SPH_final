#ifndef __SPHSYSTEM_H__
#define __SPHSYSTEM_H__

#include "sph_type.h"

class Particle
{
public:
	uint id;
	float2 pos;
	float2 vel;
	float2 acc;
	float2 ev;

	float dens;
	float pres;

	float surf_norm;

	Particle *next;
};

class SPHSystem
{
public:
	uint max_particle;
	uint num_particle;

	float kernel;
	float mass;
	float cutoff_ratio;

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
	float surf_norm;
	float surf_coe;

	float poly6_value;
	float spiky_value;
	float visco_value;

	float grad_poly6;
	float lplc_poly6;

	float kernel_2;
	float self_dens;
	float self_lplc_color;

	Particle *mem;
	Particle **cell;

	uint sys_running;

public:
	SPHSystem(float input_cutoff_ratio);
	~SPHSystem();
	void animation();
	void init_system();
	void add_particle(float2 pos, float2 vel);

private:
	void build_table();
	void comp_dens_pres();
	void comp_force_adv();
	void advection();

private:
	int2 calc_cell_pos(float2 p);
	uint calc_cell_hash(int2 cell_pos);
};

#endif
