#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

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

NarrowBridge::NarrowBridge(int travel_time, int max_wait, int id) : turn0(this), turn1(this) {
    this->travel_time = travel_time;
    this->max_wait = max_wait;
    this->id = id;
    this->passed_before = false;
    direction = 0;
    on_bridge_0 = 0;
    on_bridge_1 = 0;
}

void NarrowBridge::pass_bridge_0(Direction Direction, int car_id) {
    // printf("PASS BRIDGE 0 car id %d\n", car_id);
    __synchronized__;
    // printf("Continue BRIDGE 0 car id %d\n", car_id);
    // direction opposite 
    if (direction == 1) {
        w0.push(car_id);
        while ((w0.front() != car_id) || (direction == 1) ) {
            // printf("BUSY WAITING 0\n");
            mutex.unlock();
            turn0.wait();
            mutex.lock();
        }
    }
    else { // same direction
        w0.push(car_id);
        while ((w0.front() != car_id) || (direction == 1)) { // not first
            // printf("BUSY WAITING 1\n");
            mutex.unlock();
            turn0.wait();
            mutex.lock();
        }
    }
    // pass bridge
    if (passed_before) {
        sleep_milli(PASS_DELAY);
    }
    on_bridge_0++;
    w0.pop();
    WriteOutput(car_id, 'N', id, START_PASSING);
    passed_before = true;
    turn0.notifyAll();
}

void NarrowBridge::pass_bridge_1(Direction Direction, int car_id) {
    __synchronized__;
    if (direction == 0) {
        w1.push(car_id);
        while ((w1.front() != car_id) || (direction == 0))  {
            // printf("BUSY WAITING 2 car id %d\n", car_id);
            mutex.unlock(); 
            turn1.wait();
            mutex.lock();
        }
    }
    else {
        w1.push(car_id);
        while ((w1.front() != car_id) || (direction == 0)) {
            // printf("BUSY WAITING 3\n");
            mutex.unlock();
            turn1.wait();
            mutex.lock();
        }
    }
    if (passed_before) {
        sleep_milli(PASS_DELAY);
    }
    on_bridge_1++;
    w1.pop();
    WriteOutput(car_id, 'N', id, START_PASSING);
    passed_before = true;
    turn1.notifyAll();
}

void NarrowBridge::leave_bridge_0(int car_id) {
    // __synchronized__;
    on_bridge_0--;
    WriteOutput(car_id, 'N', id, FINISH_PASSING);
    if (on_bridge_0 == 0) {
        direction = 1;
        passed_before = false; 
        if (!w1.empty()) {
            turn1.notifyAll();
        }
    }
    else {
        turn0.notifyAll();
    }
}

void NarrowBridge::leave_bridge_1(int car_id) {
    // __synchronized__;
    on_bridge_1--;
    WriteOutput(car_id, 'N', id, FINISH_PASSING);
    if (on_bridge_1 == 0) {
        direction = 0;
        passed_before = false; 
        if (!w0.empty()) {
            turn0.notifyAll();
        }
    }
    else {
        turn1.notifyAll();
    }
    
}