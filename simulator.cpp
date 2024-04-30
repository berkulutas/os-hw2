#include "helper.h"
#include "WriteOutput.h"

int main() {
    InitWriteOutput();
    sleep_milli(99);
    WriteOutput(1, 'A', 1, TRAVEL);
    WriteOutput(2, 'B', 2, ARRIVE);
    WriteOutput(3, 'C', 3, START_PASSING);
    WriteOutput(4, 'D', 4, FINISH_PASSING);
    return 0;
}