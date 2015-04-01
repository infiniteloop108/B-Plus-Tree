all: code.cpp
	g++ code.cpp -o btree.out
clean:
	rm -f run.out
	rm -f nodes/*
	rm -f data/*
