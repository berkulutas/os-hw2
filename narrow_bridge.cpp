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
    this->on_bridge[0] = 0;
    this->on_bridge[1] = 0;
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
            if ((on_bridge[other_direction] > 0)) {
                // printf("car %d waiting for other direction to pass\n", car_id);
                turns[my_direction].wait();
                // printf("car %d waken up at %llu\n", car_id, GetTimestamp());
            }
            else if ((queues[my_direction].front() == car_id) and (my_direction == this->direction)) { // first car on queue
                if (on_bridge[my_direction] > 0) { // car passing a bridge pass delay
                    // printf("car %d waiting for pass delay at %llu\n", car_id, GetTimestamp());
                    timespec ts;
                    milli_to_abs_time(PASS_DELAY, &ts);
                    auto x = delay.timedwait(&ts);
                    // printf("return val = %d\n", x); 
                }
                // after pass delay direction same then pass
                if (my_direction == this->direction) {
                    on_bridge[my_direction]++;
                    queues[my_direction].pop();
                    timespec ts;
                    milli_to_abs_time(this->travel_time, &ts);
                    turns[my_direction].notifyAll();
                    WriteOutput(car_id, 'N', this->id, START_PASSING);
                    travel.timedwait(&ts);
                    WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
                    on_bridge[my_direction]--;
                    // printf("on_bridge[mydir] = %d, queues[my_direction].empty() %d, mydirection %d, this->direction %d\n", on_bridge[my_direction], queues[my_direction].empty(), my_direction , this->direction);
                    if((on_bridge[my_direction] == 0)) {
                        // printf("notified direction %d\n", other_direction);
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
                // printf("car %d waken up at %llu\n", car_id, GetTimestamp());
            }
        }
        // direction not same and empty lane
        else if ( !queues[my_direction].empty() and (on_bridge[my_direction] == 0) and (on_bridge[other_direction] == 0)) {
            // printf("EMPTY LANE car %d changed direction from %d to %d\n", car_id, this->direction, my_direction);
            timer_started = false;
            this->direction = my_direction;
            delay.notifyAll(); // edge case pass delay car should be notified
            turns[my_direction].notifyAll();
            turns[other_direction].notifyAll();
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
                    delay.notifyAll(); // edge case pass delay car should be notified
                    turns[my_direction].notifyAll();
                    turns[other_direction].notifyAll();
                    continue;
                }
            }
            else {
                turns[my_direction].wait();
                // printf("car %d waken up at %llu\n", car_id, GetTimestamp());
            }
        }
    }
}