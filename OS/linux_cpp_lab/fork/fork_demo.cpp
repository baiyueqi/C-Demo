#include "fork_demo.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

void runForkDemo() {
    std::cout << "Before fork, PID = " << getpid() << std::endl;

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "fork failed\n";
        return;
    }

    if (pid == 0) {
        std::cout << "[Child] PID = " << getpid()
                  << ", PPID = " << getppid() << std::endl;

        sleep(2);
        std::cout << "[Child] exit\n";
    } else {
        std::cout << "[Parent] PID = " << getpid()
                  << ", Child PID = " << pid << std::endl;

        wait(nullptr);
        std::cout << "[Parent] exit\n";
    }
}