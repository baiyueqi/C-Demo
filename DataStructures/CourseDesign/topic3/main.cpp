#include "TrafficConsultSystem.h"
#include <iostream>

int main() {
    try {
        TrafficConsultSystem system;
        system.initialize();
        system.run();
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}