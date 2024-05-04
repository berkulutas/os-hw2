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

class NarrowBridge: public Monitor {
    Condition turn0;
    Condition turn1;
    Condition delay;
    Condition travel;
    int max_wait;
    std::queue<int> q0, q1;
    int direction;
    int on_bridge_0; 
    int on_bridge_1;
    bool timer_started; 
public: 
    int travel_time;
    int id;
    bool passed_before;
public:
    NarrowBridge(int travel_time, int max_wait, int id)
    : turn0(this), turn1(this), delay(this), travel(this)
    {
        this->travel_time = travel_time;
        this->max_wait = max_wait;
        this->id = id;
        this->on_bridge_0 = 0;
        this->on_bridge_1 = 0;
        this->timer_started = false;
        this->passed_before = false;
    }
    void pass_bridge(Direction direction, int car_id);
}; 


void NarrowBridge::pass_bridge(Direction direction, int car_id) {
    __synchronized__;
    // car direction 0 -> 1
    if (direction.from == 0) {
        q0.push(car_id);
        printf("car %d is waiting\n", car_id);
        timespec ts;
        milliseconds_to_absolute_timespec(5000, &ts);
        turn0.timedwait(&ts);
        printf("car %d is passing\n", car_id);

        /*
        
        // opposite direction has the right of way
        if (this->direction == 1) {
            // empty road or no cars waiting
            // TODO change direction

            // there are cars on opposite direction
            // wait for them to pass start timer 
        }

        // same direction has the right of way
        else {
            // empty road or no cars waiting
            if (on_bridge_0 == 0) {
                on_bridge_0++;
                q0.pop();
                WriteOutput(car_id, 'N', this->id, START_PASSING);
                timespec ts;
                milliseconds_to_absolute_timespec(travel_time, &ts);
                travel.timedwait(&ts);
                WriteOutput(car_id, 'N', this->id, FINISH_PASSING);
            }
            // there are cars on the road
            else {
                // wait for the car to pass
            }
        }

        */
    }
    // car direction 1 -> 0
    
}