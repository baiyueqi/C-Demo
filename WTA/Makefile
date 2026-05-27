CXX ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic

TARGET := wta_demo
SRC := \
	src/main.cpp \
	src/data_generator.cpp \
	src/evaluator.cpp \
	src/experiment_runner.cpp \
	src/greedy_solver.cpp \
	src/genetic_solver.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
