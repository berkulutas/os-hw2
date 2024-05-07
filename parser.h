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
    Condition turns[2];
    Condition delay;
    Condition travel;
    std::vector<std::queue<int>> queues;
    int max_wait;
    int direction;
    int on_bridge;
    bool timer_started; 
public: 
    int travel_time;
    int id;
public:
    NarrowBridge(int travel_time, int max_wait, int id); 
    void pass_bridge(Direction direction, int car_id);
}; 


class Ferry: public Monitor {
    Condition full[2];
    Condition travel;
    int travel_time;
    int max_wait;
    int capacity;
    int on_ferry[2];
public:
    int id;
public:
    Ferry(int travel_time, int max_wait, int capacity, int id);
    void pass_ferry(Direction direction, int car_id);
};


class Crossroad: public Monitor {
    Condition turns[4];
    Condition delay;
    Condition travel;
    int travel_time;
    int max_wait;
    int id; 
    int direction;
    int on_crossroad;
    bool timer_started;
    std::vector<std::queue<int>> queues;
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

void milli_to_abs_time(int milliseconds, struct timespec *abs_time);

#endif //PARSER_H

