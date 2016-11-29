bakesim: bakesim.cc
	g++ bakesim.cc -o bakesim -lsimlib

run:
	LD_LIBRARY_PATH=/usr/local/lib/ ./bakesim
