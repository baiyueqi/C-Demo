CXX = clang++
CXXFLAGS = -pthread -std=c++17

TARGET = main

OBJS = main.o clinic.o ticket.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp common.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clinic.o: clinic.cpp common.h
	$(CXX) $(CXXFLAGS) -c clinic.cpp

ticket.o: ticket.cpp common.h
	$(CXX) $(CXXFLAGS) -c ticket.cpp

clean:
	rm -f *.o $(TARGET)