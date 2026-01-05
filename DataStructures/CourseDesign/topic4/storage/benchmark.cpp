#include <iostream>
#include <chrono>
#include "storage/storage.h"

int main() {
    StorageEngine engine;
    const int N = 100000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        engine.set("key" + std::to_string(i), "value");
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "SET " << N << " ops cost "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";
}