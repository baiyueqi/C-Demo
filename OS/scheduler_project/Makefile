CXX = clang++

CXXFLAGS = -pthread -std=c++17

TARGET = scheduler

OBJS = src/main.o src/scheduler.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

src/main.o: src/main.cpp include/scheduler.h
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o

src/scheduler.o: src/scheduler.cpp include/scheduler.h include/pcb.h
	$(CXX) $(CXXFLAGS) -c src/scheduler.cpp -o src/scheduler.o

run:
	./$(TARGET)

clean:
	rm -f src/*.o $(TARGET)