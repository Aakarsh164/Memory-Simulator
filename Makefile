CXX=g++
CXXFLAGS=-std=c++17 -O2 -Iinclude
SRC=$(wildcard src/*.cpp src/*/*.cpp)
OBJ=$(SRC:.cpp=.o)
BIN=bin/memsim

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
