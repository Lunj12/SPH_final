#include "sph_system.h"
#include "sph_system.cuh"

SPHSystem::SPHSystem()
{
	hParam=new SysParam();

	hParam->max_particle=10000;
	hParam->num_particle=0;
	//hParam->max_bc_particle=10000;
	hParam->num_particle_fluid=0;
	hParam->num_particle_bc=0;
	//hParam->max_body_particle=1000;
	hParam->num_particle_body=0;
  hParam->num=0;

	hParam->kernel=0.04f;
	hParam->mass=0.02f;

	hParam->world_size=make_float2(1.28f, 1.28f);
	hParam->cell_size=hParam->kernel;
	hParam->center.x=hParam->world_size.x/2;
	hParam->center.y=hParam->world_size.y*2/3;
	hParam->radi=0.04f;
	hParam->grid_size.x=(uint)ceil(hParam->world_size.x/hParam->cell_size);
	hParam->grid_size.y=(uint)ceil(hParam->world_size.y/hParam->cell_size);
	hParam->tot_cell=hParam->grid_size.x*hParam->grid_size.y;

	hParam->gravity.x=0.0f;
	hParam->gravity.y=-0.9f;
	hParam->wall_damping=-0.5f;
	hParam->rest_density=1000.0f;
	hParam->gas_constant=3.0f;
	hParam->viscosity=3.5f;
	hParam->time_step=0.003f;
	hParam->surf_normal=6.0f;
	hParam->surf_coe=0.2f;

	hParam->poly6_value=315.0f/(64.0f * PI * pow(hParam->kernel, 9));;
	hParam->spiky_value=-45.0f/(PI * pow(hParam->kernel, 6));
	hParam->visco_value=45.0f/(PI * pow(hParam->kernel, 6));

	hParam->grad_poly6=-945/(32 * PI * pow(hParam->kernel, 9));
	hParam->lplc_poly6=-945/(8 * PI * pow(hParam->kernel, 9));

	hParam->kernel_2=hParam->kernel*hParam->kernel;
	hParam->self_dens=hParam->mass*hParam->poly6_value*pow(hParam->kernel, 6);
	hParam->self_lplc_color=hParam->lplc_poly6*hParam->mass*(hParam->kernel_2)*(0-3/4*(hParam->kernel_2));

	sys_running=0;

	// dParam
	alloc_array((void**)&(dParam), sizeof(SysParam));

	hMem=(Particle *)malloc(sizeof(Particle)*hParam->max_particle);
	alloc_array((void**)&(dMem), sizeof(Particle)*hParam->max_particle);
	hPoints=(float2 *)malloc(sizeof(float2)*hParam->max_particle);
	alloc_array((void**)&(dPoints), sizeof(float2)*hParam->max_particle);

	//hBC=(Particle *)malloc(sizeof(Particle)*hParam->max_bc_particle);
	//alloc_array((void**)&(dBC), sizeof(Particle)*hParam->max_bc_particle);
	//hBODY=(Particle *)malloc(sizeof(Particle)*hParam->max_body_particle);
	//salloc_array((void**)&(dBODY), sizeof(Particle)*hParam->max_body_particle);
	//hPoints=(float2 *)malloc(sizeof(float2)*hParam->max_body_particle);
	//alloc_array((void**)&(dPoints), sizeof(float2)*hParam->max_body_particle);

	hHash=(uint *)malloc(sizeof(uint)*hParam->max_particle);
	hStart=(uint *)malloc(sizeof(uint)*hParam->tot_cell);

	alloc_array((void**)&dHash, sizeof(uint)*hParam->max_particle);
	alloc_array((void**)&dIndex, sizeof(uint)*hParam->max_particle);
	alloc_array((void**)&dStart, sizeof(uint)*hParam->tot_cell);
	alloc_array((void**)&dEnd, sizeof(uint)*hParam->tot_cell);

	printf("Initialize SPH:\n");
	printf("World Width : %f\n", hParam->world_size.x);
	printf("World Height: %f\n", hParam->world_size.y);
	printf("Cell Size  : %f\n", hParam->cell_size);
	printf("Grid Width : %u\n", hParam->grid_size.x);
	printf("Grid Height: %u\n", hParam->grid_size.y);
	printf("Poly6 Kernel: %f\n", hParam->poly6_value);
	printf("Spiky Kernel: %f\n", hParam->spiky_value);
	printf("Visco Kernel: %f\n", hParam->visco_value);
	printf("Self Density: %f\n", hParam->self_dens);
}

SPHSystem::~SPHSystem()
{
	free(hMem);
	free(hParam);
	free(hPoints);
	//free(hBC);
	//free(hBODY);
	free(hHash);
	free(hStart);

	free_array(dMem);
	free_array(dHash);
	free_array(dParam);
	free_array(dIndex);
	free_array(dStart);
	free_array(dEnd);
	free_array(dPoints);
	free_array(dBC);
	free_array(dBODY);
}

void SPHSystem::animation()
{
	if(sys_running != 1)
	{
		return;
	}

	//set_parameters(hParam);

  calc_hash(dHash, dIndex, dMem, hParam->num_particle, dParam);
	//copy_array(hHash,dHash, sizeof(uint)*hParam->num_particle, CUDA_DEV_TO_HOST);
	//printf("hHash_before=%d\n", hHash[50]);
	sort_particles(dHash, dIndex, hParam->num_particle);
	//copy_array(hHash, dHash, sizeof(uint)*hParam->num_particle, CUDA_DEV_TO_HOST);
	//printf("hHash_after=%d\n", hHash[1793]);

	find_start_end(dStart, dEnd, dHash, dIndex, hParam->num_particle, hParam->tot_cell);
	//copy_array(hStart, dStart, sizeof(uint)*hParam->tot_cell, CUDA_DEV_TO_HOST);
	//printf("tot_cell=%d;hStart=%d\n", hParam->tot_cell,hStart[400]);

	integrate_velocity(dMem, hParam->num_particle_fluid, dParam);
	compute(dMem, dHash, dIndex, dStart, dEnd, hParam->num_particle, hParam->tot_cell, dParam);
	move_body(dMem, hParam->num_particle, hParam->num_particle_bc, dParam);
	copy_buffer(dMem, dPoints, hParam->num_particle_fluid);
	copy_array(hPoints, dPoints, sizeof(float2)*hParam->num_particle_fluid, CUDA_DEV_TO_HOST);
	copy_array(hMem, dMem, sizeof(Particle)*hParam->num_particle_fluid, CUDA_DEV_TO_HOST);
	copy_array(hParam, dParam, sizeof(SysParam), CUDA_DEV_TO_HOST);
}

void SPHSystem::add_particle(float2 pos, float2 vel, Particle *list, uint &num )
{
	Particle *p=&(list[num]);

	p->pos=pos;
	p->vel=vel;
	p->acc=make_float2(0.0f, 0.0f);
	p->ev=make_float2(0.0f, 0.0f);

	p->dens=hParam->rest_density;
	p->pres=0.0f;

	num++;
	//printf("NUM=%d\n", num);
}

inline float frand()
{
    return rand() / (float) RAND_MAX;
}

void SPHSystem::init_system()
{
	srand((unsigned int)time(NULL));

	float2 pos;
	float2 vel=make_float2(0.0f, 0.0f);

	for(pos.x=hParam->world_size.x*0.0f+hParam->kernel*0.75f; pos.x<hParam->world_size.x*1.0f-hParam->kernel*0.25f; pos.x+=(hParam->kernel*0.45f))
	{
		for(pos.y=hParam->world_size.y*0.0f+hParam->kernel; pos.y<hParam->world_size.y*0.4f-hParam->kernel*0.25f; pos.y+=(hParam->kernel*0.45f))
		{
			add_particle(pos, vel, hMem, hParam->num_particle);
			//printf("posx=%f| posy=%f\n", pos.x, pos.y);
		}
	}
	//printf("posx=%f| posy=%f\n", hMem[100].pos.x, hMem[100].pos.y);
	hParam->num_particle_fluid=hParam->num_particle;
	copy_array(dMem, hMem, sizeof(Particle)*hParam->num_particle, CUDA_HOST_TO_DEV);
	copy_array(dParam, hParam, sizeof(SysParam), CUDA_HOST_TO_DEV);
	printf("number of fluid particles=%d\n",hParam->num_particle);
}


	void SPHSystem::init_boundary()
	{
		srand((unsigned int)time(NULL));

		float2 pos;
		float2 vel=make_float2(0.0f, 0.0f);
		float2 para;
		para.x=hParam->world_size.x;
		para.y=hParam->world_size.y;
		float kernel=hParam->kernel;
		for(pos.x=para.x*0.0f+kernel/2; pos.x<para.x*1.0f-kernel/2; pos.x+=(kernel*0.2f))
 	{
		pos.y=para.x*0.0f+kernel/2;
		add_particle(pos, vel, hMem, hParam->num_particle);
	}

	for(pos.x=para.x*0.0f+kernel/2; pos.x<para.x*1.0f-kernel/2; pos.x+=(kernel*0.2f))
	{
		pos.y=para.x*1.0f-kernel/2;
		add_particle(pos, vel, hMem, hParam->num_particle);
 	}

	for(pos.y=para.y*0.0f+kernel/2; pos.y<para.y*1.0f-kernel/2; pos.y+=(kernel*0.2f))
	{
		pos.x=para.x*0.0f+kernel/2;
		add_particle(pos, vel, hMem, hParam->num_particle);
 	}

	for(pos.y=para.y*0.0f+kernel/2; pos.y<para.y*1.0f-kernel/2; pos.y+=(kernel*0.2f))
	{
		pos.x=para.x*1.0f-kernel/2;
		add_particle(pos, vel, hMem, hParam->num_particle);
 	}


	for(pos.x=para.x*0.0f+kernel/2; pos.x<para.x*1.0f-kernel/2; pos.x+=(kernel*0.2f))
 {
	 pos.y=para.x*0.0f+kernel/3;
	 add_particle(pos, vel, hMem, hParam->num_particle);
 }

 for(pos.x=para.x*0.0f+kernel/2; pos.x<para.x*1.0f-kernel/2; pos.x+=(kernel*0.2f))
 {
	 pos.y=para.x*1.0f-kernel/3;
	 add_particle(pos, vel, hMem, hParam->num_particle);
 }

 for(pos.y=para.y*0.0f+kernel/2; pos.y<para.y*1.0f-kernel/2; pos.y+=(kernel*0.2f))
 {
	 pos.x=para.x*0.0f+kernel/3;
	 add_particle(pos, vel, hMem, hParam->num_particle);
 }

 for(pos.y=para.y*0.0f+kernel/2; pos.y<para.y*1.0f-kernel/2; pos.y+=(kernel*0.2f))
 {
	 pos.x=para.x*1.0f-kernel/3;
	 add_particle(pos, vel, hMem, hParam->num_particle);
 }
	// alloc_array((void**)&(dParam), sizeof(SysParam));
	hParam->num_particle_bc=hParam->num_particle;
	copy_array(dMem, hMem, sizeof(Particle)*hParam->num_particle, CUDA_HOST_TO_DEV);
	copy_array(dParam, hParam, sizeof(SysParam), CUDA_HOST_TO_DEV);
	//printf("Number pf Boundary Particles: %u\n", hParam->num_particle);
	printf("Number pf Boundary Particles: %u\n", hParam->num_particle_bc);
}

void SPHSystem::init_body()
{
	srand((unsigned int)time(NULL));

	float2 pos;
	float2 vel=make_float2(0.0f, 0.0f);
	float2 center;
	center.x=hParam->center.x;
	center.y=hParam->center.y;
	float radi=hParam->radi;
	float kernel=hParam->kernel;

	for(float theta=0; theta<=2*PI; theta+=(PI/100))
	{
	    pos.x=center.x+radi*cos(theta);
			pos.y=center.y+radi*sin(theta);
			add_particle(pos, vel, hMem, hParam->num_particle);
	}
	hParam->num_particle_body=hParam->num_particle;
	copy_array(dMem, hMem, sizeof(Particle)*hParam->num_particle, CUDA_HOST_TO_DEV);
	copy_array(dParam, hParam, sizeof(SysParam), CUDA_HOST_TO_DEV);
	//printf("Number pf Boundary Particles: %u\n", hParam->num_particle);
	printf("Number pf BOdy Particles: %u\n", hParam->num_particle_body);

}
