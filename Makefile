CXX = g++
CXXFLAGS = -std=c++17 -Wall

SRC = src/main.cpp src/allocator/allocator.cpp
OUT = memsim

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f memsim
