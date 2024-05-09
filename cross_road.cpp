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
    this->on_crossroad[0] = 0;
    this->on_crossroad[1] = 0;
    this->on_crossroad[2] = 0;
    this->on_crossroad[3] = 0;
    this->timer_started = false;
    queues.resize(4);
}


void Crossroad::pass_crossroad(Direction direction, int car_id) {
    __synchronized__;
    int my_direction = direction.from;
    // add car to queue, keep track of order
    queues[my_direction].push(car_id);
    
    while (1) {
        // same direction
        if (direction.from == this->direction) {
            if (on_crossroad[(my_direction+1)%4] or on_crossroad[(my_direction+2)%4] or on_crossroad[(my_direction+3)%4]) {
                // printf("car %d waiting for other direction to pass\n", car_id);
                turns[my_direction].wait();
                // printf("car %d waken up at %llu\n", car_id, GetTimestamp());
            }
            else if ((queues[my_direction].front() == car_id)) { // first car on queue
                // printf("direction %d que front %d, car_id %d\n", my_direction, queues[my_direction].front(), car_id);
                if (on_crossroad[my_direction] > 0) { // if a car is passing bridge in same direction
                    timespec ts;
                    // printf("car %d waiting for pass delay at %llu\n", car_id, GetTimestamp());
                    milli_to_abs_time(PASS_DELAY, &ts); // wait pass delay
                    // printf("car %d is waiting for pass delay in direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                    delay.timedwait(&ts);
                    // printf("car %d wake up from pass delay in direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                }
                if (my_direction == this->direction) {
                    on_crossroad[my_direction]++;
                    queues[my_direction].pop();
                    turns[my_direction].notifyAll();
                    WriteOutput(car_id, 'C', this->id, START_PASSING);
                    timespec ts;
                    turns[my_direction].notifyAll();
                    milli_to_abs_time(this->travel_time, &ts);
                    travel.timedwait(&ts);
                    WriteOutput(car_id, 'C', this->id, FINISH_PASSING);
                    on_crossroad[my_direction]--;
                    // if direction already changed, notify it
                    if ((this->direction != my_direction) and (on_crossroad[my_direction] == 0)) {
                        turns[this->direction].notifyAll();
                    }
                    // direction same but no more cars, notify incrementally next direction waiting cars
                    else if ((queues[my_direction].empty() and on_crossroad[my_direction] == 0)) {
                        // find first busy next direction
                        for (int i = 1; i < 4; i++) {
                            int next_direction = (this->direction + i) % 4;
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
                turns[my_direction].wait();
            }

        }
        // no cars in passing lane 
        else if ((on_crossroad[my_direction] == 0) and (on_crossroad[(my_direction+1)%4] == 0) and (on_crossroad[(my_direction+2)%4] == 0) and (on_crossroad[(my_direction+3)%4] == 0) ) {
            // printf("EMPTY LANE car %d changed direction from %d to %d\n", car_id, this->direction, my_direction);
            this->direction = my_direction;
            timer_started = false;
            delay.notifyAll(); // edge case pass delay car should be notified
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
                    // printf("car %d is timed out on direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                    int prev_direction = this->direction; // prev direction of crossroad
                    // find first busy next direction
                    for (int i = 1; i < 4; i++) {
                        int next_direction = (this->direction + i) % 4;
                        if (queues[next_direction].empty() == false) {
                            this->direction = next_direction;
                            timer_started = false;
                            delay.notifyAll(); // edge case pass delay car should be notified
                            for (int i = 0; i < 4; i++) {
                                turns[i].notifyAll();
                            }
                            // printf("direction changed from %d to %d, at time %llu\n", prev_direction, next_direction, GetTimestamp());
                            break;
                        }
                        // printf("NO BUSY NEXT ROAD FOUND!!!! 1\n");
                    }
                }
            }
            else {
                turns[my_direction].wait();
            }

        }
    }
}