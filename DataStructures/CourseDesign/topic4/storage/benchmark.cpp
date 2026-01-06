#include "storage.h"
#include <chrono>
#include <iostream>

int main() {
    StorageEngine engine;
    const int N = 100000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        engine.set("key" + std::to_string(i), "value");
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "SET QPS: "
              << N / std::chrono::duration<double>(end - start).count()
              << std::endl;
}
