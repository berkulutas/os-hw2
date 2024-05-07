#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

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
        milli_to_abs_time(this->travel_time, &ts);
        on_ferry[my_direction] = 0;
        full[my_direction].notifyAll();
        WriteOutput(car_id, 'F', this->id, START_PASSING);
        travel.timedwait(&ts);
        WriteOutput(car_id, 'F', this->id, FINISH_PASSING);
    }
    else {
        timespec ts;
        milli_to_abs_time(this->max_wait, &ts);
        if (full[my_direction].timedwait(&ts) == ETIMEDOUT) {
            on_ferry[my_direction] = 0; 
            full[my_direction].notifyAll();
        }
        timespec ts2;
        milli_to_abs_time(this->travel_time, &ts2);
        WriteOutput(car_id, 'F', this->id, START_PASSING);
        travel.timedwait(&ts2);
        WriteOutput(car_id, 'F', this->id, FINISH_PASSING);
    } 

}