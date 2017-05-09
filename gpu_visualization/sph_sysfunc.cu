#include "sph_header.h"
#include "sph_system.h"
#include "sph_math.h"
#include "cutil_math.h"

void alloc_array(void **dev_ptr, size_t size)
{
    cudaMalloc(dev_ptr, size);
}

void free_array(void *dev_ptr)
{
    cudaFree(dev_ptr);
}

void copy_array(void *ptr_a, void *ptr_b, size_t size, int type)
{
	if(type == 1)
	{
		cudaMemcpy(ptr_a, ptr_b, size, cudaMemcpyHostToDevice);
		return;
	}

	if(type == 2)
	{
		cudaMemcpy(ptr_a, ptr_b, size, cudaMemcpyDeviceToHost);
		return;
	}

	if(type == 3)
	{
		cudaMemcpy(ptr_a, ptr_b, size, cudaMemcpyDeviceToDevice);
		return;
	}

	return;
}

void compute_grid_size(uint num_particle, uint block_size, uint &num_blocks, uint &num_threads)
{
    num_threads=min(block_size, num_particle);
    num_blocks=iDivUp(num_particle, num_threads);
}

__global__
void copy_buffer_kernel(Particle *dMem, float2 *buffer, uint num_particle)
{
	uint index=blockIdx.x*blockDim.x+threadIdx.x;

    if(index >= num_particle)
	{
		return;
	}

	buffer[index].x=dMem[index].pos.x;
	buffer[index].y=dMem[index].pos.y;
}

void copy_buffer(Particle *dMem, float2 *buffer, uint num_particle)
{
	if(num_particle == 0)
	{
		return;
	}

	uint num_threads;
	uint num_blocks;

    compute_grid_size(num_particle, 256, num_blocks, num_threads);

	copy_buffer_kernel<<< num_blocks, num_threads >>>(dMem, buffer, num_particle);
}

__device__
int2 calc_grid_pos(float2 p, SysParam* dParam)
{
	int2 grid_pos;
    grid_pos.x = floor((p.x) / dParam->cell_size);
    grid_pos.y = floor((p.y) / dParam->cell_size);

    return grid_pos;
}

__device__ uint calc_grid_hash(int2 grid_pos, SysParam* dParam)
{
	if(grid_pos.x<0 || grid_pos.x>=dParam->grid_size.x || grid_pos.y<0 || grid_pos.y>=dParam->grid_size.y)
	{
		return (uint)0xffffffff;
	}

	grid_pos.x = grid_pos.x & (dParam->grid_size.x-1);
    grid_pos.y = grid_pos.y & (dParam->grid_size.y-1);

	return ((uint)(grid_pos.y))*dParam->grid_size.x+(uint)(grid_pos.x);
}

__global__
void calc_hashK(uint *dHash, uint *dIndex, Particle *dMem, uint num_particle, SysParam* dParam)
{
    uint index=blockIdx.x*blockDim.x+threadIdx.x;

    if(index >= num_particle)
	{
		return;
	}

	int2 grid_pos=calc_grid_pos(dMem[index].pos, dParam);
    uint hash=calc_grid_hash(grid_pos, dParam);

    dHash[index]=hash;
    dIndex[index]=index;
}

void calc_hash(uint *dHash, uint *dIndex, Particle *dMem, uint num_particle, SysParam* dParam)
{
	if(num_particle == 0)
	{
		return;
	}

    uint num_threads;
	  uint num_blocks;

    compute_grid_size(num_particle, 256, num_blocks, num_threads);

	calc_hashK<<< num_blocks, num_threads >>>(dHash, dIndex, dMem, num_particle, dParam);
}

void sort_particles(uint *dHash, uint *dIndex, uint num_particle)
{
	if(num_particle == 0)
	{
		return;
	}

    thrust::sort_by_key(thrust::device_ptr<uint>(dHash),
                        thrust::device_ptr<uint>(dHash + num_particle),
                        thrust::device_ptr<uint>(dIndex));
                        //key_begin,key_end,values
}

__global__
void find_start_end_kernel(uint *dStart, uint *dEnd, uint *dHash, uint *dIndex, uint num_particle)
{
	extern __shared__ uint shared_hash[];
    uint index=blockIdx.x*blockDim.x+threadIdx.x;

    uint hash;

    if(index < num_particle)
	{
        hash=dHash[index];
	    shared_hash[threadIdx.x+1]=hash;

	    if(index > 0 && threadIdx.x == 0)
	    {
		    shared_hash[0]=dHash[index-1];
	    }
	}

	__syncthreads();

	if(index < num_particle)
	{
		if(index == 0 || hash != shared_hash[threadIdx.x])
	    {
		    dStart[hash]=index;

            if(index > 0)
			         {
                dEnd[shared_hash[threadIdx.x]]=index;
			         }
	    }

        if (index == num_particle-1)
        {
            dEnd[hash]=index+1;
        }
	}
}

void find_start_end(uint *dStart, uint *dEnd, uint *dHash, uint *dIndex, uint num_particle, uint num_cell)
{
	if(num_particle == 0)
	{
		return;
	}

    uint num_thread;
	  uint num_block;
    compute_grid_size(num_particle, 256, num_block, num_thread);

    cudaMemset(dStart, 0xffffffff, num_cell*sizeof(int));
	cudaMemset(dEnd, 0x0, num_cell*sizeof(int));

    uint smemSize=sizeof(int)*(num_thread+1);

    find_start_end_kernel<<< num_block, num_thread, smemSize>>>(dStart, dEnd, dHash, dIndex, num_particle);
}

__global__
void integrate_velocity_kernel(Particle* dMem, uint num_particle_fluid, SysParam* dParam)
{
	uint index=blockIdx.x*blockDim.x+threadIdx.x;

	if(index >= num_particle_fluid)
	{
		return;
	}

	Particle *p=&(dMem[index]);

	p->vel=p->vel+p->acc*dParam->time_step/p->dens+dParam->gravity*dParam->time_step;
	p->pos=p->pos+p->vel*dParam->time_step;

	if(p->pos.x >= dParam->world_size.x-BOUNDARY)
	{
		p->vel.x=p->vel.x*dParam->wall_damping;
		p->pos.x=dParam->world_size.x-BOUNDARY;
	}

	if(p->pos.x < 0.0f)
	{
		p->vel.x=p->vel.x*dParam->wall_damping;
		p->pos.x=0.0f;
	}

	if(p->pos.y >= dParam->world_size.y-BOUNDARY)
	{
		p->vel.y=p->vel.y*dParam->wall_damping;
		p->pos.y=dParam->world_size.y-BOUNDARY;
	}

	if(p->pos.y < 0.0f)
	{
		p->vel.y=p->vel.y*dParam->wall_damping;
		p->pos.y=0.0f;
	}

	p->ev=(p->ev+p->vel)/2;
}

void integrate_velocity(Particle *dMem, uint num_particle_fluid, SysParam* dParam)
{
	if(num_particle_fluid == 0)
	{
		return;
	}

	uint num_thread;
	uint num_block;
    compute_grid_size(num_particle_fluid, 256, num_block, num_thread);

	integrate_velocity_kernel<<< num_block, num_thread >>>(dMem, num_particle_fluid, dParam);
}

__device__
float compute_cell_density(uint index,
					int2 neighbor,
					Particle *dMem,
					uint *dHash,
					uint *dIndex,
					uint *dStart,
					uint *dEnd,
					uint num_particle,
					uint tot_cell,
          SysParam* dParam)
{
	float total_cell_density=0.0f;
	uint grid_hash=calc_grid_hash(neighbor, dParam);
	if(grid_hash == 0xffffffff)
	{
		return total_cell_density;
	}
	uint start_index=dStart[grid_hash];

	float mass=dParam->mass;
	float kernel_2=dParam->kernel_2;
	float poly6_value=dParam->poly6_value;

	float2 rel_pos;
	float r2;

	Particle *p=&(dMem[index]);
	Particle *np;
	uint neighbor_index;

	if(start_index != 0xffffffff)
	{
        uint end_index=dEnd[grid_hash];

        for(uint count_index=start_index; count_index<end_index; count_index++)
		{
			neighbor_index=dIndex[count_index];
			np=&(dMem[neighbor_index]);

			rel_pos=np->pos-p->pos;
			r2=rel_pos.x*rel_pos.x+rel_pos.y*rel_pos.y;

			if(r2<INF || r2>=kernel_2)
			{
				continue;
			}

			total_cell_density=total_cell_density + mass * poly6_value * pow(kernel_2-r2, 3);
        }
	}

	return total_cell_density;
}

__global__
void compute_density_kernel(Particle *dMem,
							uint *dHash,
							uint *dIndex,
							uint *dStart,
							uint *dEnd,
							uint num_particle,
							uint tot_cell,
              SysParam* dParam)
{
	uint index=blockIdx.x*blockDim.x+threadIdx.x;

	if(index >= num_particle)
	{
		return;
	}

	int2 cell_pos=calc_grid_pos(dMem[index].pos, dParam);

	float total_density=0;

	for(int y=-1; y<=1; y++)
	{
		for(int x=-1; x<=1; x++)
		{
			int2 neighbor_pos = cell_pos+ make_int2(x, y);
			total_density=total_density+compute_cell_density(index, neighbor_pos, dMem, dHash, dIndex, dStart, dEnd, num_particle, tot_cell, dParam);
        }
    }
	total_density=total_density+dParam->self_dens;

	if(total_density < INF)
	{
		dMem[index].dens=dParam->rest_density;
	}

	dMem[index].dens=total_density;
	dMem[index].pres=(pow(dMem[index].dens / dParam->rest_density, 7) - 1) * dParam->gas_constant;
}

__device__
float2 compute_cell_force(uint index, int2 neighbor, Particle *dMem, uint *dHash, uint *dIndex, uint *dStart, uint *dEnd, uint num_particle, uint tot_cell, float2 &grad_color, float &lplc_color, SysParam* dParam)
{
	float2 total_cell_force=make_float2(0.0f);
	uint grid_hash=calc_grid_hash(neighbor, dParam);

	if(grid_hash == 0xffffffff)
	{
		return total_cell_force;
	}

	uint start_index=dStart[grid_hash];

	float kernel=dParam->kernel;
	float mass=dParam->mass;
	float kernel_2=dParam->kernel_2;

	uint neighbor_index;

	Particle *p=&(dMem[index]);
	Particle *np;

	float2 rel_pos;
	float r2;
	float r;

	float V;
	float kernel_r;

	float pressure_kernel;
	float temp_force;

	float2 rel_vel;
	float viscosity_kernel;


	if(start_index != 0xffffffff)
	{
		uint end_index=dEnd[grid_hash];

        for(uint count_index=start_index; count_index<end_index; count_index++)
		{
			neighbor_index=dIndex[count_index];

			np=&(dMem[neighbor_index]);

			rel_pos=p->pos-np->pos;
			r2=rel_pos.x*rel_pos.x+rel_pos.y*rel_pos.y;

			if(r2 < kernel_2 && r2 > INF)
			{
				r=sqrt(r2);
				V=mass/np->dens/2;
				kernel_r=kernel-r;

				pressure_kernel=dParam->spiky_value * kernel_r * kernel_r;
				temp_force=V * (p->pres+np->pres) * pressure_kernel;
				total_cell_force=total_cell_force-rel_pos*temp_force/r;

				rel_vel=np->ev-p->ev;
				viscosity_kernel=dParam->visco_value*(kernel-r);
				temp_force=V * dParam->viscosity * viscosity_kernel;
				total_cell_force=total_cell_force + rel_vel*temp_force;

				float temp=(-1) * dParam->grad_poly6 * V * pow(kernel_2-r2, 2);
				grad_color.x += temp * rel_pos.x;
				grad_color.y += temp * rel_pos.y;
				lplc_color += dParam->lplc_poly6 * V * (kernel_2-r2) * (r2-3/4*(kernel_2-r2));
			}
        }
	}

	return total_cell_force;
}

__global__
void compute_force_kernel(Particle *dMem, uint *dHash, uint *dIndex, uint *dStart, uint *dEnd, uint num_particle, uint tot_cell, SysParam* dParam)
{
	uint index=blockIdx.x*blockDim.x+threadIdx.x;

	if(index >= num_particle)
	{
		return;
	}

	int2 cell_pos=calc_grid_pos(dMem[index].pos, dParam);

	float2 total_force=make_float2(0.0f, 0.0f);
	float2 grad_color=make_float2(0.0f);
	float lplc_color=0.0f;

	for(int y=-1; y<=1; y++)
	{
		for(int x=-1; x<=1; x++)
		{
			int2 neighbor_pos = cell_pos + make_int2(x, y);
			total_force=total_force+compute_cell_force(index, neighbor_pos, dMem, dHash, dIndex, dStart, dEnd, num_particle, tot_cell, grad_color, lplc_color, dParam);
		}
	}
	dMem[index].acc=total_force;

	lplc_color+=dParam->self_lplc_color/dMem[index].dens;
	dMem[index].surf_norm=sqrt(grad_color.x*grad_color.x+grad_color.y*grad_color.y);
	float2 force;

	if(dMem[index].surf_norm > dParam->surf_normal)
	{
		force=dParam->surf_coe * lplc_color * grad_color / dMem[index].surf_norm;
	}
	else
	{
		force=make_float2(0.0f, 0.0f);
	}

	dMem[index].acc+=force;
}

void compute(Particle *dMem, uint *dHash, uint *dIndex, uint *dStart, uint *dEnd, uint num_particle, uint tot_cell, SysParam* dParam)
{
	if(num_particle == 0)
	{
		return;
	}

	uint num_thread;
	uint num_block;
    compute_grid_size(num_particle, 256, num_block, num_thread);

	compute_density_kernel<<< num_block, num_thread >>>(dMem, dHash, dIndex, dStart, dEnd, num_particle, tot_cell, dParam);
	compute_force_kernel<<< num_block, num_thread >>>(dMem, dHash, dIndex, dStart, dEnd, num_particle, tot_cell, dParam);
}

__global__
void move_body_kernel(Particle *dMem, uint num_particle, uint num_particle_bc, SysParam* dParam)
{
  uint index=blockIdx.x*blockDim.x+threadIdx.x;
  uint num_particle_body=num_particle-num_particle_bc;

	if(index >= num_particle_body || dParam->center.y<0.2f)
	{
		return;
	}


  	Particle *p=&(dMem[index+num_particle_bc+1]);
    dParam->center.x=0.0f;
	  dParam->center.y+=-0.003f;
    p->pos.x=dParam->center.x+p->pos.x;
   	p->pos.y=-0.003f+p->pos.y;
    p->vel=make_float2(0.0f);
    p->acc=make_float2(0.0f);
    p->ev=make_float2(0.0f);
}


void move_body(Particle *dMem, uint num_particle, uint num_particle_bc, SysParam* dParam)
{
  if(num_particle==num_particle_bc){
    return;
  }

  uint num_thread;
  uint num_block;
  uint num_particle_body=num_particle-num_particle_bc;
    compute_grid_size(num_particle_body, 256, num_block, num_thread);
    move_body_kernel<<< num_block, num_thread >>>(dMem, num_particle, num_particle_bc, dParam);
}

__global__
void fix_boundary_kernel(Particle *dMem, uint num_particle, uint num_particle_fluid, SysParam* dParam)
{
  uint index=blockIdx.x*blockDim.x+threadIdx.x;

  if(index >= num_particle_fluid)
  {
    return;
  }


    Particle *p=&(dMem[index+num_particle_fluid+1]);
    p->vel=make_float2(0.0f);
    p->vel=make_float2(0.0f);
    p->acc=make_float2(0.0f);
    p->ev=make_float2(0.0f);
}

void fix_boundary(Particle *dMem, uint num_particle, uint num_particle_fluid, SysParam* dParam)
{
  if(num_particle==num_particle_fluid){
    return;
  }

  uint num_thread;
  uint num_block;
  uint num_particle_bc=num_particle-num_particle_fluid;
    compute_grid_size(num_particle_bc, 256, num_block, num_thread);
    fix_boundary_kernel<<< num_block, num_thread >>>(dMem, num_particle, num_particle_fluid, dParam);

}
