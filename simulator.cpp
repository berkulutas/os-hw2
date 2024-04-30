#include "helper.h"
#include "WriteOutput.h"
#include <iostream>
#include <iostream>
#include "parser.h"


int main() {
    std::cout << "before init" << std::endl;
    Simulation simulation;
    parse_input(&simulation);
    std::cout << "before finish" << std::endl;
}
