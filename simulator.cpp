#include <pthread.h>
#include <iostream>

#include "helper.h"
#include "WriteOutput.h"
#include "parser.h"
#include "monitor.h"


Simulation simulation;
std::vector<pthread_t> threads;


void pass_ferry(CarPathObject &path_object, int car_id) {
    // sleep for travel time
}

void pass_crossroad(CarPathObject &path_object, int car_id) {
    // sleep for travel time
}

void pass(CarPathObject &path_object, int car_id) {
    // determine type then do appropriate action
    switch (path_object.connector.type) {
        case 'N':
        {
            WriteOutput(car_id, 'N', path_object.connector.id, ARRIVE);
            NarrowBridge* bridge = &simulation.narrow_bridges[path_object.connector.id];
            if (path_object.direction.from == 0)
            {
                bridge->pass_bridge_0(path_object.direction, car_id);
                sleep_milli(bridge->travel_time);
                bridge->leave_bridge_0(car_id);
            }
            else
            {
                bridge->pass_bridge_1(path_object.direction, car_id);
                sleep_milli(bridge->travel_time);
                bridge->leave_bridge_1(car_id);
            }
            break;

        }
        case 'F':
        {
            pass_ferry(path_object, car_id);
            break;
        }
        case 'C':
        {
            pass_crossroad(path_object, car_id);
            break;
        }
        default:
        {
            break;   
        }
    }
    // sleep for travel time

}

void *car_thread(void *arg) {
    Car *car = (Car *) arg;

    for (int i = 0; i < car->path_length; i++) {
        // travel till connector
        WriteOutput(car->id, car->path[i].connector.type, car->path[i].connector.id, TRAVEL);
        sleep_milli(car->travel_time);
        // try to pass connector
        pass(car->path[i], car->id);
    }
}

int main() {

    parse_input(&simulation);

    InitWriteOutput();

    // create all car threads
    for (int i = 0; i < simulation.cars_count; i++) {
        pthread_t tid;
        threads.push_back(tid);
        pthread_create(&tid, NULL, car_thread, (void *) &simulation.cars[i]);
    }
    
    // Wait for all threads to finish
    for (int i = 0; i < simulation.cars_count; i++) {
        pthread_join(threads[i], NULL);
    }

}