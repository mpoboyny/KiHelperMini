# Run from simulation/RunDirExit.
# Build: make release
# Run:   ../../bin/simulation/RunDirExit/release/RunDirExit
# Clean: make clean

CXX ?= g++

BUILD_TYPE := release
BIN_DIR := ../../bin/simulation/RunDirExit/$(BUILD_TYPE)
OBJ_DIR := ../../obj/simulation/RunDirExit/$(BUILD_TYPE)
TARGET := $(BIN_DIR)/RunDirExit
OBJECT := $(OBJ_DIR)/simulation.o
HELPERCLI := ../../bin/release/helperCli

CXXFLAGS := -std=c++17 -Wall -O3 -DNDEBUG

.PHONY: all release clean

all: release

release: $(TARGET)

$(HELPERCLI):
	$(MAKE) -C ../.. release

$(TARGET): simulation.cxx ../RunAndExit/simulation.cxx $(HELPERCLI)
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c simulation.cxx -o $(OBJECT)
	$(CXX) $(OBJECT) -o $(TARGET)

clean:
	@rm -rf ../../obj/simulation/RunDirExit ../../bin/simulation/RunDirExit
