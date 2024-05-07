#include "parser.h"
#include "helper.h"
#include "WriteOutput.h"

void milli_to_abs_time(int milliseconds, struct timespec *abs_time) {
    // get current time
    clock_gettime(CLOCK_REALTIME, abs_time);

    // add to curr time
    abs_time->tv_sec += milliseconds / 1000;
    abs_time->tv_nsec += (milliseconds % 1000) * 1000000;

    // adjust overflow 
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