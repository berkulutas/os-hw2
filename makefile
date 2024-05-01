all: simulator.cpp 
	g++ -lpthread -g -o simulator simulator.cpp parser.cpp WriteOutput.c helper.c
make clean:
	rm simulator
