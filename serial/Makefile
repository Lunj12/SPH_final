CC = g++
# CFLAGS = -O0 -g -lm
CFLAGS =
LIBS = -lm 


TARGETS = sph-serial

all:	$(TARGETS)

sph-serial: sph_system.o sph_main.o sph_common.o
	$(CC) -o $@ sph_system.o sph_main.o sph_common.o $(LIBS)

sph_system.o: sph_system.cpp
	$(CC) -c $(CFLAGS) sph_system.cpp

sph_main.o: sph_main.cpp
	$(CC) -c $(CFLAGS) sph_main.cpp

sph_common.o: sph_common.cpp
	$(CC) -c $(CFLAGS) sph_common.cpp

clean:
	rm -f *.o $(TARGETS) *.stdout *.txt
