# Run from simulation/RunDirManyExit.
# Build: make release
# Run:   ../../bin/simulation/RunDirManyExit/release/RunDirManyExit
# Clean: make clean

CXX ?= g++

BUILD_TYPE := release
BIN_DIR := ../../bin/simulation/RunDirManyExit/$(BUILD_TYPE)
OBJ_DIR := ../../obj/simulation/RunDirManyExit/$(BUILD_TYPE)
TARGET := $(BIN_DIR)/RunDirManyExit
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
	@rm -rf ../../obj/simulation/RunDirManyExit ../../bin/simulation/RunDirManyExit
