CXX = clang++
CXXFLAGS = -Wall -g -std=c++17 -pthread

TARGET = banker

OBJS = main.o banker.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp banker.h
	$(CXX) $(CXXFLAGS) -c main.cpp

banker.o: banker.cpp banker.h
	$(CXX) $(CXXFLAGS) -c banker.cpp

clean:
	rm -f *.o $(TARGET)