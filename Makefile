bakesim: bakesim.cc bakesim.hh
	$(CXX) bakesim.cc -o $@ -std=c++11 -lsimlib

run: bakesim
	LD_LIBRARY_PATH=/usr/local/lib/ ./bakesim

show: run
	less bakesim.out

docs.pdf: docs.tex
	pdflatex $^

clean:
	rm bakesim
