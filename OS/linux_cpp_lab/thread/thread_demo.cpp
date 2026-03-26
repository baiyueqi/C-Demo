#include "thread_demo.h"
#include <iostream>
#include <thread>
#include <chrono>

void task1() {
    for (int i = 0; i < 5; i++) {
        std::cout << "[Thread 1] i = " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void task2() {
    for (int i = 0; i < 5; i++) {
        std::cout << "[Thread 2] i = " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void runThreadDemo() {
    std::thread t1(task1);
    std::thread t2(task2);

    t1.join();
    t2.join();

    std::cout << "All threads finished\n";
}