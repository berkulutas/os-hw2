#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

void parse_input(Simulation *simulation) {
    int NN;
    int FN;
    int CN;

    int CAN;

    // first line contains the number of narrow bridges (NN) in the simulation
    std::cin >> NN;
    // The following NN lines contain the properties of the narrow bridge with ith ID (All IDs start from 0) in the following format:
    for (int i = 0; i < NN; i++) {
        int nt, nm;;
        std::cin >> nt >> nm;
        simulation->narrow_bridges.push_back(NarrowBridge(nt, nm, i));
    }
    
    // The next line contains the number of ferries (FN ) in the simulation
    std::cin >> FN;
    // the following FN lines contain the properties of the ferries with ith ID in the following format: - FT FM FC where
    for (int i = 0; i < FN; i++) {
        int ft, fm, fc;
        std::cin >> ft >> fm >> fc;
        simulation->ferries.push_back({ft, fm, fc});
    }

    // The next line contains the number of crossroads (CN ) in the simulation.
    std::cin >> CN;
    // Following CN lines contain the properties of the crossroads with ith ID in the following format:- CT CM
    for (int i = 0; i < CN; i++) {
        int ct, cm;
        std::cin >> ct >> cm;
        simulation->crossroads.push_back({ct, cm});
    }

    // The next line contains the number of cars (CAN ) in the simulation.
    std::cin >> CAN;
    // Following CAN × 2 lines
    // contain the properties of the cars with ith ID in the following (two line) format:
    // - CAT CAP
    // - PC1 FC1 TC1 ... PCN FCN TCN
    for (int i = 0; i < CAN; i++) {
        int cat, cap;
        std::cin >> cat >> cap;
        Car car;
        car.id = i;
        car.travel_time = cat;
        car.path_length = cap;
        for (int j = 0; j < cap; j++) {
            std::string pc;
            int fc, tc;
            std::cin >> pc >> fc >> tc;
            car.path.push_back({parse_pc(pc), {fc, tc}});
        }
        simulation->cars.push_back(car);
    }

    simulation->narrow_bridges_count = NN;
    simulation->ferries_count = FN;
    simulation->crossroads_count = CN;
    simulation->cars_count = CAN;
}

ConnectorObject parse_pc(std::string pc) {
    char type = pc[0];
    int id = std::stoi(pc.substr(1));
    return {type, id};
}

NarrowBridge::NarrowBridge(int travel_time, int max_wait, int id) : turn0(this), turn1(this) {
    this->travel_time = travel_time;
    this->max_wait = max_wait;
    this->id = id;
    this->passed_before = false;
    direction = 0;
    on_bridge_0 = 0;
    on_bridge_1 = 0;
    timer_started = false;
}

void NarrowBridge::pass_bridge(Direction direction, int car_id) {
    __synchronized__;
    // car direction 0 -> 1
    if (direction.from == 0) {
        q0.push(car_id);
        while(1) {
            // opposite directions
            if (direction.from != this->direction) {
                // bridge has cars
                if (on_bridge_1 > 0) {
                    if (!timer_started) {
                        timer_started = true;
                        // start timer when it exceeds direction change
                        // turn0.timedwait((timespec*)this->max_wait); // TODO
                        // if no cars in the opposite direction
                        // change direction
                        // while (on_bridge_1 > 0) {
                        //     turn0.wait();
                        // }
                        // this->direction = 1;
                        // printf("DIRECTION CHANGERD 0\n");
                        // passed_before = false;
                        // timer_started = false;
                        // turn0.notifyAll();
                        
                    }
                    else {
                        // wait for the other cars to pass
                        while (q0.front() != car_id) {
                            turn0.wait();
                        }
                    }
                }
                // no cars in the opposite direction
                else {
                    // change direction
                    this->direction = 1;
                    printf("DIRECTION CHANGERD 1 \n");
                    passed_before = false;
                    timer_started = false;
                    turn0.notifyAll();
                }
            }
            // same direction
            if (!passed_before) {
                // printf("ILK ARABA 0 car id %d\n", car_id);
                passed_before = true;
                // start passing
                WriteOutput(car_id, 'N', this->id, START_PASSING);
                q0.pop();
                on_bridge_0++;
                turn0.notifyAll();
                // mutex.unlock();
                sleep_milli(this->travel_time);
                // mutex.lock();
                // finish passing
                WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                on_bridge_0--;
                if (on_bridge_0 == 0) {
                    turn1.notifyAll();
                }
                break;
            }
            else {
                // printf("ILK DEGIL 0 car id %d\n", car_id);
                // wait for the other cars to pass
                while (q0.front() != car_id) {
                    turn0.wait();
                }
                mutex.unlock();
                sleep_milli(PASS_DELAY);
                mutex.lock();
                // direction has changed
                if (this->direction != 0) {
                    continue;
                }
                // direction same pass the bridge
                // start passing
                WriteOutput(car_id, 'N', this->id, START_PASSING);
                q0.pop();
                on_bridge_0++;
                turn0.notifyAll();
                // mutex.unlock();
                sleep_milli(this->travel_time);
                // mutex.lock();
                // finish passing
                WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                on_bridge_0--;
                if (on_bridge_0 == 0) {
                    turn1.notifyAll();
                }
                break;
            }

        }

    }
    // car direction 1 -> 0
    else {
        q1.push(car_id);
        while(1) {
            // opposite directions
            if (direction.from != this->direction) {
                // bridge has cars
                if (on_bridge_0 > 0) {
                    if (!timer_started) {
                        timer_started = true;
                        // start timer when it exceeds direction change
                        // turn1.timedwait((timespec *)this->max_wait); // TODO
                        // if no cars in the opposite direction
                        // change direction
                        // while (on_bridge_0 > 0) {
                        //     turn1.wait();
                        // }
                        // this->direction = 0;
                        // printf("DIRECTION CHANGERD 2 \n");
                        // passed_before = false;
                        // timer_started = false;
                        // turn1.notifyAll();
                        
                    }
                    else {
                        // wait for the other cars to pass
                        while (q1.front() != car_id) {
                            turn1.wait();
                        }
                    }
                }
                // no cars in the opposite direction
                else {
                    // change direction
                    this->direction = 0;
                    printf("DIRECTION CHANGERD 3 \n");
                    passed_before = false;
                    timer_started = false;
                    turn1.notifyAll();
                }
            }
            // same direction
            if (!passed_before) {
                // printf("ILK ARABA 0 car id %d\n", car_id);
                passed_before = true;
                // start passing
                WriteOutput(car_id, 'N', this->id, START_PASSING);
                q1.pop();
                on_bridge_1++;
                turn1.notifyAll();
                // mutex.unlock();
                sleep_milli(this->travel_time);
                // mutex.lock();
                // finish passing
                WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                on_bridge_1--;
                if (on_bridge_1 == 0) {
                    turn0.notifyAll();
                }
                break;
            }
            else {
                // printf("ILK DEGIL 1 car id %d\n", car_id);
                // wait for the other cars to pass
                while (q1.front() != car_id) {
                    turn1.wait();
                }
                mutex.unlock();
                sleep_milli(PASS_DELAY);
                mutex.lock();
                // direction has changed
                if (this->direction != 1) {
                    continue;
                }
                // direction same pass the bridge
                // start passing
                WriteOutput(car_id, 'N', this->id, START_PASSING);
                q1.pop();
                on_bridge_1++;
                turn1.notifyAll();
                // mutex.unlock();
                sleep_milli(this->travel_time);
                // mutex.lock();
                // finish passing
                WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                on_bridge_1--;
                if (on_bridge_1 == 0) {
                    turn0.notifyAll();
                }
                break;
            }

        }

    }
}