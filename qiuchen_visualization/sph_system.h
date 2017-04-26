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
	float2 ev; //

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
	uint max_bc_particle;
	uint num_particle_bc;
	uint max_body_particle;
	uint num_particle_body;
	uint num;

	float kernel;
	float mass;

	float2 world_size;
	float2 center;
	float radi;
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
	Particle *BC;
	Particle *BODY;
	Particle *list;
	Particle **cell_body;

	uint sys_running;

public:
	SPHSystem();
	~SPHSystem();
	void animation();
	void init_system();
	void init_boundary();
	void init_body();
	void add_particle(float2 pos, float2 vel, Particle *list, uint &num);

private:
	void build_table();
	void build_table_body();
	void comp_dens_pres();
	void comp_force_adv();
	void comp_dens_pres_body();
	void advection();
	void move_body();

private:
	int2 calc_cell_pos(float2 p);
	uint calc_cell_hash(int2 cell_pos);
};

#endif
