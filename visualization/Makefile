CC = g++
# CFLAGS = -O0 -g -lm
CFLAGS =
LIBS = -lm -lGL -lGLU -lglut


TARGETS = sph_vis

all:	$(TARGETS)

sph_vis: sph_system.o sph_main.o sph_timer.o sph_common.o
	$(CC) -o $@ sph_system.o sph_main.o sph_timer.o sph_common.o $(LIBS)

sph_system.o: sph_system.cpp
	$(CC) -c $(CFLAGS) sph_system.cpp

sph_main.o: sph_main.cpp
	$(CC) -c $(CFLAGS) sph_main.cpp

sph_timer.o: sph_timer.cpp
	$(CC) -c $(CFLAGS) sph_timer.cpp

sph_common.o: sph_common.cpp
	$(CC) -c $(CFLAGS) sph_common.cpp


clean:
	rm -f *.o $(TARGETS) *.stdout *.txt
