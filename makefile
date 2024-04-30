all: simulator.cpp 
	g++ -o simulator simulator.cpp WriteOutput.c helper.c

make clean:
	rm simulator
