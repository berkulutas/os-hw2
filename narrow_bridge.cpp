#include "helper.h"
#include "parser.h"
#include "WriteOutput.h"

// NARROW BRIDGE
NarrowBridge::NarrowBridge(int travel_time, int max_wait, int id) 
: turns{this, this}, delay(this), travel(this)
{
    this->travel_time = travel_time;
    this->max_wait = max_wait;
    this->id = id;
    this->on_bridge = 0; 
    this->direction = 0;
    this->timer_started = false;
    queues.resize(2);
    // queues[0] = std::queue<int>();
    // queues[1] = std::queue<int>();
}

void NarrowBridge::pass_bridge(Direction direction, int car_id) {
    __synchronized__;
    int my_direction = direction.from;
    int other_direction = (my_direction + 1) % 2;
    // add car to queue, keep track of order
    this->queues[my_direction].push(car_id);

    while (1) {
        // same direction
        if (my_direction == this->direction) {
            if ((on_bridge > 0) and queues[my_direction].front() != car_id) {
                turns[my_direction].wait();
            }
            else if ((queues[my_direction].front() == car_id)) { // first car on queue
                if (on_bridge > 0) { // car passing a bridge pass delay
                    timespec ts;
                    milli_to_abs_time(PASS_DELAY, &ts);
                    delay.timedwait(&ts);
                }
                // after pass delay direction same then pass
                if (my_direction == this->direction) {
                    on_bridge++;
                    queues[my_direction].pop();
                    timespec ts;
                    milli_to_abs_time(this->travel_time, &ts);
                    turns[my_direction].notifyAll();
                    WriteOutput(car_id, 'N', this->id, START_PASSING);
                    travel.timedwait(&ts);
                    WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                    on_bridge--;
                    if((on_bridge == 0) and (queues[my_direction].empty() or (my_direction != this->direction))) {
                        turns[other_direction].notifyAll();
                    }
                    break; 
                }
                // after pass dealy direction changed then wait
                else {
                    // printf("while pass delay direction changed car_id = %d\n", car_id); 
                    continue; // direction changed
                }
            }
            else {
                turns[my_direction].wait();
            }
        }
        else if ( !queues[my_direction].empty() and (on_bridge == 0) ) {
            timer_started = false;
            this->direction = my_direction;
            turns[my_direction].notifyAll();
        }
        // different direction
        else {
            if (timer_started == false) {
                timer_started = true;
                // printf("car %d started timer at %llu\n", car_id, GetTimestamp());
                timespec ts;
                milli_to_abs_time(this->max_wait, &ts);
                if (turns[my_direction].timedwait(&ts) == ETIMEDOUT) {
                    // printf("car %d timed out at %llu\n", car_id, GetTimestamp());
                    timer_started = false;
                    this->direction = my_direction;
                    turns[my_direction].wait();
                    turns[my_direction].notifyAll();
                    continue;
                }
            }
            else {
                turns[my_direction].wait();
            }
        }
    }
}