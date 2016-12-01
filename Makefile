bakesim: bakesim.cc
	g++ bakesim.cc -o bakesim -lsimlib

run: bakesim
	LD_LIBRARY_PATH=/usr/local/lib/ ./bakesim
