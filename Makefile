all: code.cpp
	g++ code.cpp -o btree.out
clean:
	rm -f nodes/*
	rm -f data/*
	rm -f *.out
