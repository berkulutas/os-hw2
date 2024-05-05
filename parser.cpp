#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

void milliseconds_to_absolute_timespec(int milliseconds, struct timespec *abs_time) {
    // Get current time
    clock_gettime(CLOCK_REALTIME, abs_time);

    // Add milliseconds to the current time
    abs_time->tv_sec += milliseconds / 1000;
    abs_time->tv_nsec += (milliseconds % 1000) * 1000000;

    // Adjust if nanoseconds overflow into seconds
    if (abs_time->tv_nsec >= 1000000000) {
        abs_time->tv_sec++;
        abs_time->tv_nsec -= 1000000000;
    }
}

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
        auto ptr = new NarrowBridge(nt, nm, i);
        simulation->narrow_bridges.push_back(ptr);
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

NarrowBridge::NarrowBridge(int travel_time, int max_wait, int id) 
: turn0(this), turn1(this), travel(this), delay(this)
{
    this->travel_time = travel_time;
    this->max_wait = max_wait;
    this->id = id;
    this->on_bridge_0 = 0;
    this->on_bridge_1 = 0;
    this->direction = 0;
    this->timer_started = false;
}


void NarrowBridge::pass_bridge(Direction direction, int car_id) {
    __synchronized__;
    if (direction.from == 0) q0.push(car_id);
    else q1.push(car_id);

    while (1) {
        // same direction
        if (direction.from == this->direction) {
            // car direction 0 -> 1
            if (direction.from == 0) {
                if ((on_bridge_0 > 0) and (q0.front() != car_id)) {
                    turn0.wait();
                }
                else if ((q0.front() == car_id) and (on_bridge_1 == 0)) { // first car on queue
                    if (on_bridge_0 > 0) { // if a car is passing bridge
                        timespec ts;
                        milliseconds_to_absolute_timespec(PASS_DELAY, &ts);
                        delay.timedwait(&ts);
                    }
                    if (direction.from == this->direction) {
                        on_bridge_0++;
                        q0.pop();
                        WriteOutput(car_id, 'N', this->id, START_PASSING);
                        timespec ts;
                        turn0.notifyAll();
                        milliseconds_to_absolute_timespec(this->travel_time, &ts);
                        auto retval = travel.timedwait(&ts);
                        WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                        on_bridge_0--;
                        if ((q0.empty() and on_bridge_0 == 0) or (this->direction == 1)) {
                            turn1.notifyAll();
                        }
                        break; // car passed
                    }
                    else {
                        printf("while pass delay direction changed car_id = %d\n", car_id); 
                        continue; // direction changed
                    }
                }
                else {
                    turn0.wait();
                }
            }
            // car direction 1 -> 0
            else {
                if ((on_bridge_1 > 0) and (q1.front() != car_id)) {
                    turn1.wait();
                }
                else if ((q1.front() == car_id) and (on_bridge_0 == 0)) { // first car on queue
                    if (on_bridge_1 > 0) { // if a car is passing bridge
                        timespec ts;
                        milliseconds_to_absolute_timespec(PASS_DELAY, &ts);
                        delay.timedwait(&ts);
                    }
                    if (direction.from == this->direction) {
                        on_bridge_1++;
                        q1.pop();
                        WriteOutput(car_id, 'N', this->id, START_PASSING);
                        timespec ts;
                        turn1.notifyAll();
                        milliseconds_to_absolute_timespec(this->travel_time, &ts);
                        auto retval = travel.timedwait(&ts);
                        WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                        on_bridge_1--;
                        if ((q1.empty() and on_bridge_1 == 0) or (this->direction == 0)) {
                            turn0.notifyAll();
                        }
                        break; // car passed
                    }
                    else {
                        printf("while pass delay direction changed car_id = %d\n", car_id); 
                        continue; // direction changed
                    }
                }
                else {
                    turn1.wait();
                }
            }
        }
        else if ( (this->direction == 0) and (q0.empty()) and (on_bridge_0 == 0) ) {
            this->direction = 1;
            timer_started = false;
            printf("YOL BOS anam direction changed from 0 to 1\n");
            turn1.notifyAll();
        }
        else if ( (this->direction == 1) and (q1.empty()) and (on_bridge_1 == 0) ) {
            this->direction = 0;
            timer_started = false; 
            printf("YOL BOS anam  direction changed from 1 to 0\n");
            turn0.notifyAll();
        }
        else {
            // different direction
            if (direction.from == 0) {
                if (timer_started == false) {
                    timer_started = true;
                    printf("car %d is the first car in queue0\n", car_id);
                    timespec ts;
                    milliseconds_to_absolute_timespec(this->max_wait, &ts);
                    if (turn0.timedwait(&ts) == ETIMEDOUT) {
                        this->direction = 0;
                        timer_started = false;
                        turn0.notifyAll();
                        continue;
                    }
                }
                else {
                    turn0.wait();
                }
            }
            else {
                if (timer_started == false) {
                    timer_started = true; 
                    printf("car %d is the first car in queue1\n", car_id);
                    timespec ts;
                    milliseconds_to_absolute_timespec(this->max_wait, &ts);
                    if (turn1.timedwait(&ts) == ETIMEDOUT) {
                        printf("TIMEOUT happened\n");
                        this->direction = 1;
                        printf("direction changed from 0 to 1\n");
                        timer_started = false;
                        turn1.notifyAll();
                        continue;
                    }

                }
                else {
                    turn1.wait();
                }
            }
        }

    }
}

