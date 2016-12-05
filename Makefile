bakesim: bakesim.cc bakesim.hh
	$(CXX) bakesim.cc -o $@ -lsimlib

run: bakesim
	LD_LIBRARY_PATH=/usr/local/lib/ ./bakesim
