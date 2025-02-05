# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
LDFLAGS = -lstdc++fs  # Link against the C++ standard library

# Source and output files
SRC = sobel_edge_detection.cpp
TARGET = sobel_edge_detector

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
