CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

TARGET = minijudge
SRC = src/main.cpp src/compiler.cpp src/runner.cpp src/comparator.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf judge_tmp

.PHONY: all clean
