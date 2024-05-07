#include "helper.h"
#include "parser.h"
#include "WriteOutput.h"

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
                        milli_to_abs_time(PASS_DELAY, &ts);
                        delay.timedwait(&ts);
                    }
                    if (direction.from == this->direction) {
                        on_bridge_0++;
                        q0.pop();
                        WriteOutput(car_id, 'N', this->id, START_PASSING);
                        timespec ts;
                        turn0.notifyAll();
                        milli_to_abs_time(this->travel_time, &ts);
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
                        milli_to_abs_time(PASS_DELAY, &ts);
                        delay.timedwait(&ts);
                    }
                    if (direction.from == this->direction) {
                        on_bridge_1++;
                        q1.pop();
                        WriteOutput(car_id, 'N', this->id, START_PASSING);
                        timespec ts;
                        turn1.notifyAll();
                        milli_to_abs_time(this->travel_time, &ts);
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
                    milli_to_abs_time(this->max_wait, &ts);
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
                    milli_to_abs_time(this->max_wait, &ts);
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