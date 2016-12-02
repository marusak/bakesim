bakesim: bakesim.cc bakesim.hh
	g++ bakesim.cc bakesim.hh -o bakesim -lsimlib

run: bakesim
	LD_LIBRARY_PATH=/usr/local/lib/ ./bakesim
