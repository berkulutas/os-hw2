#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>

typedef struct Direction {
    int from;
    int to;
} Direction;

typedef struct NarrowBridge {
    int travel_time;
    int max_wait;
} NarrowBridge;

typedef struct Ferry {
    int travel_time;
    int max_wait;
    int capacity;
} Ferry;

typedef struct Crossroad {
    int travel_time;
    int max_wait;
} Crossroad;

typedef struct ConnectorObject {
    char type;
    int id;
} ConnectorObject; 

typedef struct CarPathObject {
    ConnectorObject connector;
    Direction direction;
} CarPathObject;

typedef struct Car {
    int id;
    int travel_time; 
    int path_length;
    std::vector<CarPathObject> path;
} Car;

typedef struct Simulation {
    int narrow_bridges_count;
    int ferries_count;
    int crossroads_count;
    int cars_count;
    
    std::vector<NarrowBridge> narrow_bridges;
    std::vector<Ferry> ferries;
    std::vector<Crossroad> crossroads;
    std::vector<Car> cars;
} Simulation;

void parse_input(Simulation *simulation);

ConnectorObject parse_pc(std::string pc);

#endif //PARSER_H

