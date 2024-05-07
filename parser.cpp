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
        auto ptr = new Ferry(ft, fm, fc, i);
        simulation->ferries.push_back(ptr);
    }

    // The next line contains the number of crossroads (CN ) in the simulation.
    std::cin >> CN;
    // Following CN lines contain the properties of the crossroads with ith ID in the following format:- CT CM
    for (int i = 0; i < CN; i++) {
        int ct, cm;
        std::cin >> ct >> cm;
        auto ptr = new Crossroad(ct, cm, i);
        simulation->crossroads.push_back(ptr);
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

// NARROW BRIDGE

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
                        // printf("while pass delay direction changed car_id = %d\n", car_id); 
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
                        // printf("while pass delay direction changed car_id = %d\n", car_id); 
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
            // printf("YOL BOS direction changed from 0 to 1\n");
            turn1.notifyAll();
        }
        else if ( (this->direction == 1) and (q1.empty()) and (on_bridge_1 == 0) ) {
            this->direction = 0;
            timer_started = false; 
            // printf("YOL BOS direction changed from 1 to 0\n");
            turn0.notifyAll();
        }
        else {
            // different direction
            if (direction.from == 0) {
                if (timer_started == false) {
                    timer_started = true;
                    // printf("car %d is the first car in queue0\n", car_id);
                    timespec ts;
                    milliseconds_to_absolute_timespec(this->max_wait, &ts);
                    if (turn0.timedwait(&ts) == ETIMEDOUT) {
                        this->direction = 0;
                        timer_started = false;
                        // wait for last car to pass
                        turn0.wait();
                        turn0.notifyAll();
                        turn1.notifyAll();
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
                    // printf("car %d is the first car in queue1\n", car_id);
                    timespec ts;
                    milliseconds_to_absolute_timespec(this->max_wait, &ts);
                    if (turn1.timedwait(&ts) == ETIMEDOUT) {
                        // printf("TIMEOUT happened\n");
                        this->direction = 1;
                        timer_started = false;
                        // printf("direction changed from 0 to 1\n");
                        // wait for last car to pass
                        turn1.wait();
                        turn1.notifyAll();
                        turn0.notifyAll();
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

// FERRY
Ferry::Ferry(int travel_time, int max_wait, int capacity, int id)
: travel(this), full{this, this}
{
    this->travel_time = travel_time;
    this->max_wait = max_wait;
    this->capacity = capacity;
    this->on_ferry[0] = 0;
    this->on_ferry[1] = 0;
    this->id = id;
}

void Ferry::pass_ferry(Direction direction, int car_id) {
    __synchronized__;
    
    int my_direction = direction.from;
    on_ferry[my_direction]++; 

    if (on_ferry[my_direction] == this->capacity) {
        timespec ts;
        milliseconds_to_absolute_timespec(this->travel_time, &ts);
        on_ferry[my_direction] = 0;
        full[my_direction].notifyAll();
        WriteOutput(car_id, 'F', this->id, START_PASSING);
        travel.timedwait(&ts);
        WriteOutput(car_id, 'F', this->id, FINISH_PASSING);
    }
    else {
        timespec ts;
        milliseconds_to_absolute_timespec(this->max_wait, &ts);
        if (full[my_direction].timedwait(&ts) == ETIMEDOUT) {
            on_ferry[my_direction] = 0; 
            full[my_direction].notifyAll();
        }
        timespec ts2;
        milliseconds_to_absolute_timespec(this->travel_time, &ts2);
        WriteOutput(car_id, 'F', this->id, START_PASSING);
        travel.timedwait(&ts2);
        WriteOutput(car_id, 'F', this->id, FINISH_PASSING);
    } 

}

// CROSSROAD
Crossroad::Crossroad(int travel_time, int max_wait, int id)
: travel(this), delay(this), turns{this, this, this, this}
{
    this->travel_time = travel_time;
    this->max_wait = max_wait;
    this->id = id;
    this->direction = 0;
    this->on_crossroad = 0;
    this->timer_started = false;
    queues.resize(4);
    queues[0] = std::queue<int>();
    queues[1] = std::queue<int>();
    queues[2] = std::queue<int>();
    queues[3] = std::queue<int>();
}

void Crossroad::pass_crossroad(Direction direction, int car_id) {
    __synchronized__;
    int my_direction = direction.from;
    // add car to queue, keep track of order
    queues[my_direction].push(car_id);

    while (1) {
        // same direction
        if (direction.from == this->direction) {
            if ((on_crossroad > 0) and (queues[my_direction].front() != car_id)) {
                turns[my_direction].wait();
            }
            else if ((queues[my_direction].front() == car_id)) { // first car on queue
                // printf("direction %d que front %d, car_id %d\n", my_direction, queues[my_direction].front(), car_id);
                if (on_crossroad > 0) { // if a car is passing bridge in same direction
                    timespec ts;
                    milliseconds_to_absolute_timespec(PASS_DELAY, &ts); // wait pass delay
                    // printf("car %d is waiting for pass delay in direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                    delay.timedwait(&ts);
                }
                if (my_direction == this->direction) {
                    on_crossroad++;
                    queues[my_direction].pop();
                    WriteOutput(car_id, 'C', this->id, START_PASSING);
                    timespec ts;
                    turns[my_direction].notifyAll();
                    milliseconds_to_absolute_timespec(this->travel_time, &ts);
                    travel.timedwait(&ts);
                    WriteOutput(car_id, 'C', this->id, FINISH_PASSING);
                    on_crossroad--;
                    // if direction already changed, notify it
                    if ((this->direction != my_direction) and (on_crossroad == 0)) {
                        turns[this->direction].notifyAll();
                    }
                    // direction same but no more cars, notify incrementally next direction waiting cars
                    else if ((queues[my_direction].empty() and on_crossroad == 0)) {
                        // find first busy next direction
                        for (int i = 1; i < 4; i++) {
                            int next_direction = (my_direction + i) % 4;
                            if (queues[next_direction].empty() == false) {
                                this->direction = next_direction;
                                timer_started = false;
                                turns[next_direction].notifyAll();
                                break;
                            }
                            // printf("NO BUSY NEXT ROAD FOUND!!!! 0\n");
                        }
                    }
                    break; // car passed
                }
                else {
                    // printf("while pass delay direction changed car_id = %d\n", car_id);
                    continue; // direction changed
                }
            }
            else {
                // printf("bu waite hic giriyor mu car_id = %d\n", car_id); 
                turns[my_direction].wait();
            }

    }
        // no cars in passing lane 
        else if (on_crossroad == 0) {
            this->direction = my_direction;
            timer_started = false;
            turns[my_direction].notifyAll();
        }
        // different direction
        else {
            if (timer_started == false) {
                // printf("car %d is started timer on direction%d, at time %llu\n", car_id, my_direction, GetTimestamp());
                timer_started = true;
                timespec ts;
                milliseconds_to_absolute_timespec(this->max_wait, &ts);
                if (turns[my_direction].timedwait(&ts) == ETIMEDOUT) {
                    // printf("TIMEOUT happened at %llu\n", GetTimestamp());
                    int prev_direction = this->direction; // prev direction of crossroad
                    // find first busy next direction
                    for (int i = 1; i < 4; i++) {
                        int next_direction = (this->direction + i) % 4;
                        if (queues[next_direction].empty() == false) {
                            this->direction = next_direction;
                            // printf("direction changed from %d to %d, at time %llu\n", prev_direction, next_direction, GetTimestamp());
                            break;
                        }
                        // printf("NO BUSY NEXT ROAD FOUND!!!! 1\n");
                    }
                    // wait for last car to pass
                    // printf("car %d is waiting for last car to pass in direction %d, at time %llu\n", car_id,prev_direction, GetTimestamp());
                    turns[this->direction].wait();
                    timer_started = false;
                    // printf("last car passed in direction %d, at time %llu\n", prev_direction, GetTimestamp());
                    // notify all directions
                    for (int i = 0; i < 4; i++) {
                        turns[i].notifyAll();
                    }
                }
            }
            else {
                turns[my_direction].wait();
            }

        }
    }
}