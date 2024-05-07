#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

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
                    milli_to_abs_time(PASS_DELAY, &ts); // wait pass delay
                    // printf("car %d is waiting for pass delay in direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                    delay.timedwait(&ts);
                }
                if (my_direction == this->direction) {
                    on_crossroad++;
                    queues[my_direction].pop();
                    WriteOutput(car_id, 'C', this->id, START_PASSING);
                    timespec ts;
                    turns[my_direction].notifyAll();
                    milli_to_abs_time(this->travel_time, &ts);
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
                milli_to_abs_time(this->max_wait, &ts);
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