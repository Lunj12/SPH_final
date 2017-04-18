#include "sph_header.h"
#include "sph_system.h"

SPHSystem *sph;

void  init_sph_system()
{
	sph=new SPHSystem();
	sph->init_system();
}

int main(int argc, char **argv)
{

	init_sph_system();

	free(sph);

	return 0;
}
