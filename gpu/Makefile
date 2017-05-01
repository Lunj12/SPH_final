# Load CUDA using the following command
# module load cuda
#
CC = nvcc
CFLAGS = -O3 -arch=compute_35 -code=sm_35
NVCCFLAGS = -O3 -arch=compute_35 -code=sm_35
NVCCLIBS = -O3 -arch=compute_35 -code=sm_35
LIBS =

TARGETS = sph-gpu

all:	$(TARGETS)

sph-gpu: sph_kernel.o sph_main.o sph_system.o sph_sysfunc.o sph_common.o
	$(CC) -o $@ $(NVCCLIBS) sph_kernel.o sph_main.o sph_system.o sph_sysfunc.o sph_common.o

sph_kernel.o: sph_kernel.cu
	$(CC) -c $(NVCCFLAGS) sph_kernel.cu
sph_main.o: sph_main.cpp
	$(CC) -c $(CFLAGS) sph_main.cpp
sph_system.o: sph_system.cpp
	$(CC) -c $(CFLAGS) sph_system.cpp
sph_sysfunc.o: sph_sysfunc.cu
	$(CC) -c $(NVCCFLAGS) sph_sysfunc.cu
sph_common.o: sph_common.cpp
	$(CC) -c $(CFLAGS) sph_common.cpp

clean:
	rm -f *.o $(TARGETS) *.stdout *.txt
