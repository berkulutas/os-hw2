#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

// CROSSROAD
Crossroad::Crossroad(int travel_time, int max_wait, int id)
: travel(this), delay(this), turn(this)
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

    while(1) {
        // same direction
        if (my_direction == this->direction) {
            // if there is a car on crossroad in other direction
            if (on_crossroad[(my_direction+1)%4] or on_crossroad[(my_direction+2)%4] or on_crossroad[(my_direction+3)%4]) {
                // printf("car %d waiting for other direction to pass\n", car_id);
                turn.wait();
                // printf("0 car %d waken up at %llu\n", car_id, GetTimestamp());
            }
            // i am the first car on queue
            else if (queues[my_direction].front() == car_id) {
                // if there is a car on crossroad in same direction
                if (on_crossroad[my_direction] > 0) {
                    timespec ts;
                    milli_to_abs_time(PASS_DELAY, &ts);
                    // printf("car %d is waiting for pass delay in direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                    delay.timedwait(&ts);
                    // printf("car %d wake up from pass delay in direction %d, at time %llu\n", car_id, my_direction, GetTimestamp());
                }
                // if direction is still same
                if (my_direction == this->direction) {
                    on_crossroad[my_direction]++;
                    queues[my_direction].pop();
                    timespec ts;
                    milli_to_abs_time(this->travel_time, &ts);
                    turn.notifyAll();
                    WriteOutput(car_id, 'C', this->id, START_PASSING);
                    mutex.unlock();
                    // travel.timedwait(&ts);
                    sleep_milli(this->travel_time);
                    mutex.lock();
                    WriteOutput(car_id, 'C', this->id, FINISH_PASSING);
                    on_crossroad[my_direction]--;
                    
                    if ((on_crossroad[my_direction] == 0)) {
                        turn.notifyAll();
                    }
                    break; // exit loop car passed
                }
                else {
                    // printf("while waiting for pass delay, direction changed carid %d\n", car_id);
                    continue;
                }
            }
            else {
                // printf("car %d waiting for other cars to pass\n", car_id);
                turn.wait();
                // printf("1 car %d waken up at %llu\n", car_id, GetTimestamp());
            }
        }
        // no more cars lef on current direction then change next non-empty
        else if (on_crossroad[this->direction] == 0) {
            // printf("EMPTY LANE car %d changed direction from %d to %d\n", car_id, this->direction, my_direction);
            this->direction = my_direction;
            timer_started = false;
            delay.notifyAll(); // edge case 
            turn.notifyAll();
            continue;
        }
        // different direction
        else {
            // printf("car %d waiting for other direction to pass\n", car_id);
            turn.wait();
            // printf("2 car %d waken up at %llu\n", car_id, GetTimestamp());
        }
    }
}