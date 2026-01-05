#ifndef TRAFFICCONSULTSYSTEM_H
#define TRAFFICCONSULTSYSTEM_H

#include "Graph.h"
#include <string>

class TrafficConsultSystem {
private:
    Graph graph;
    bool dataLoaded;
    
    std::string getCityInput(const std::string& prompt);
    int getIntInput(const std::string& prompt, int min, int max);
    int getAlgorithmChoice();
    
public:
    TrafficConsultSystem();
    void initialize();
    void run();
    void showMenu();
    
    void handleFunction1();
    void handleFunction2();
    void handleFunction3();
    void handleFunction4();
};

#endif