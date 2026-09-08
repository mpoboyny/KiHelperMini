PARENT_DIR := ..
include $(PARENT_DIR)/common.mk

TARGET_NAME := safe_ldd
BIN_DIR_ROOT := $(PARENT_DIR)/bin
TARGET := $(BIN_DIR_ROOT)/release/tools/$(TARGET_NAME)

SRC := safe_ldd.cxx

.PHONY: all release clean echo

all: release
release: $(TARGET)

$(TARGET): $(SRC)
	@echo "Building $(TARGET)..."
	@mkdir -p $(dir $(TARGET))
	$(CXX) $(COMMON_CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	@echo "Clean..."
	rm -f $(TARGET)

echo:
	@echo "Available phonies:"
	@echo "  all release clean echo"
	@echo
	@echo "Variables:"
	@echo "  PARENT_DIR=$(PARENT_DIR)"
	@echo "  TARGET_NAME=$(TARGET_NAME)"
	@echo "  BIN_DIR_ROOT=$(BIN_DIR_ROOT)"
	@echo "  TARGET=$(TARGET)"
	@echo "  SRC=$(SRC)"
	@echo "  OS_DEF=$(OS_DEF)"
	@echo "  CXX=$(CXX)"
	@echo "  COMMON_CXXFLAGS=$(COMMON_CXXFLAGS)"
