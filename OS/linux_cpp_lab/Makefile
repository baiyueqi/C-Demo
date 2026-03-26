CXX = clang++
CXXFLAGS = -Wall -g -std=c++17 -pthread

all: fork_demo thread_demo

# ---------- fork ----------
fork_demo: fork/fork_demo.o fork/main.o
	$(CXX) $(CXXFLAGS) -o fork_demo fork/fork_demo.o fork/main.o

fork/fork_demo.o: fork/fork_demo.cpp fork/fork_demo.h
	$(CXX) $(CXXFLAGS) -c fork/fork_demo.cpp -o fork/fork_demo.o

fork/main.o: fork/main.cpp fork/fork_demo.h
	$(CXX) $(CXXFLAGS) -c fork/main.cpp -o fork/main.o

# ---------- thread ----------
thread_demo: thread/thread_demo.o thread/main.o
	$(CXX) $(CXXFLAGS) -o thread_demo thread/thread_demo.o thread/main.o

thread/thread_demo.o: thread/thread_demo.cpp thread/thread_demo.h
	$(CXX) $(CXXFLAGS) -c thread/thread_demo.cpp -o thread/thread_demo.o

thread/main.o: thread/main.cpp thread/thread_demo.h
	$(CXX) $(CXXFLAGS) -c thread/main.cpp -o thread/main.o

clean:
	rm -f fork/*.o thread/*.o fork_demo thread_demo