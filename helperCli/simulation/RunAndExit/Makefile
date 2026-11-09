# Run from simulation/RunAndExit.
# Build: make release
# Run:   ../../bin/simulation/RunAndExit/release/RunAndExit
# Clean: make clean

CXX ?= g++

BUILD_TYPE := release
BIN_DIR := ../../bin/simulation/RunAndExit/$(BUILD_TYPE)
OBJ_DIR := ../../obj/simulation/RunAndExit/$(BUILD_TYPE)
TARGET := $(BIN_DIR)/RunAndExit
OBJECT := $(OBJ_DIR)/simulation.o
HELPERCLI := ../../bin/release/helperCli

CXXFLAGS := -std=c++17 -Wall -O3 -DNDEBUG

.PHONY: all release clean

all: release

release: $(TARGET)

$(HELPERCLI):
	$(MAKE) -C ../.. release

$(TARGET): simulation.cxx $(HELPERCLI)
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c simulation.cxx -o $(OBJECT)
	$(CXX) $(OBJECT) -o $(TARGET)

clean:
	@rm -rf ../../obj/simulation/RunAndExit ../../bin/simulation/RunAndExit
