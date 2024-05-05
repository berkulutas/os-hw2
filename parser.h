#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include "monitor.h"

typedef struct Direction {
    int from;
    int to;
} Direction;

class NarrowBridge: public Monitor {
    Condition turn0;
    Condition turn1;
    Condition delay;
    Condition travel;
    std::queue<int> q0, q1;
    int max_wait;
    int direction;
    int on_bridge_0; 
    int on_bridge_1;
    bool timer_started; 
public: 
    int travel_time;
    int id;
public:
    NarrowBridge(int travel_time, int max_wait, int id); 
    void pass_bridge(Direction direction, int car_id);
}; 

// typedef struct Ferry {
//     int travel_time;
//     int max_wait;
//     int capacity;
// } Ferry;

class Ferry: public Monitor {
    Condition full_0, full_1;
    Condition travel;
    int travel_time;
    int max_wait;
    int capacity;
    int on_ferry_0;
    int on_ferry_1;
    std::vector<int> ferry_0;
    std::vector<int> ferry_1;
public:
    int id;
public:
    Ferry(int travel_time, int max_wait, int capacity, int id);
    void pass_ferry(Direction direction, int car_id);
};

// typedef struct Crossroad {
//     int travel_time;
//     int max_wait;
// } Crossroad;

class Crossroad: public Monitor {
    Condition delay;
    Condition travel;
    int travel_time;
    int max_wait;
    int id; 
public:
    Crossroad(int travel_time, int max_wait, int id);
    void pass_crossroad(Direction direction, int car_id);
};

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
    
    std::vector<NarrowBridge*> narrow_bridges;
    std::vector<Ferry*> ferries;
    std::vector<Crossroad*> crossroads;
    std::vector<Car> cars;
} Simulation;

void parse_input(Simulation *simulation);

ConnectorObject parse_pc(std::string pc);

#endif //PARSER_H

