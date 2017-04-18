CC = g++
# CFLAGS = -O0 -g -lm
CFLAGS =
LIBS = -lm


TARGETS = sph_naive

all:	$(TARGETS)

sph_naive: sph_system.o sph_main.o
	$(CC) -o $@ $(LIBS) sph_system.o sph_main.o

sph_system.o: sph_system.cpp
	$(CC) -c $(CFLAGS) sph_system.cpp

sph_main.o: sph_main.cpp
	$(CC) -c $(CFLAGS) sph_main.cpp

clean:
	rm -f *.o $(TARGETS) *.stdout *.txt
