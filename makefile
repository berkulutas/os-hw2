all: simulator.cpp 
	g++ -lpthread -g -o simulator simulator.cpp parser.cpp WriteOutput.c helper.c narrow_bridge.cpp ferry.cpp cross_road3.cpp
make clean:
	rm simulator
