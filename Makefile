# --- Pfade ---
BASE_DIR     := .
WX_BASE_DIR  := ./wxWidgets/3.3.1
SRC_DIR      := $(BASE_DIR)/source
INC_DIR      := $(BASE_DIR)/include
OBJ_DIR_ROOT := $(BASE_DIR)/obj
BIN_DIR_ROOT := $(BASE_DIR)/bin

# --- Konfiguration ---
TARGET_NAME := KiHelperMini
OS_DEF      := LINUX_OS

# System Libs (Reihenfolge: CUDA vor System/Math/GOMP)
SYS_LIBS   := -lpthread -lm -ldl -lrt -lX11 -lcublas -lcurand -lcudart -lcuda -lgomp

# --- Compiler Settings ---
CXX         := g++
# -fopenmp ist zwingend für die GOMP-Symbole
COMMON_CXXFLAGS := -Wall -std=c++17 -fopenmp -I$(INC_DIR) -D$(OS_DEF)

# --- PCH Settings ---
PCH_HEADER   := $(INC_DIR)/prc.hxx
PCH_GCH      := $(PCH_HEADER).gch

SOURCES := $(wildcard $(SRC_DIR)/*.cxx)

.PHONY: all debug release clean rund runr runddd echo _build .inner_link

all: release

echo:
	@echo "Available phonies:"
	@echo "  all debug release clean rund runr runddd echo"
	@echo
	@echo "Variables:"
	@echo "  BASE_DIR=$(BASE_DIR)"
	@echo "  WX_BASE_DIR=$(WX_BASE_DIR)"
	@echo "  SRC_DIR=$(SRC_DIR)"
	@echo "  INC_DIR=$(INC_DIR)"
	@echo "  OBJ_DIR_ROOT=$(OBJ_DIR_ROOT)"
	@echo "  BIN_DIR_ROOT=$(BIN_DIR_ROOT)"
	@echo "  TARGET_NAME=$(TARGET_NAME)"
	@echo "  OS_DEF=$(OS_DEF)"
	@echo "  CXX=$(CXX)"
	@echo "  COMMON_CXXFLAGS=$(COMMON_CXXFLAGS)"
	@echo "  SYS_LIBS=$(SYS_LIBS)"
	@echo "  PCH_HEADER=$(PCH_HEADER)"
	@echo "  PCH_GCH=$(PCH_GCH)"

$(PCH_GCH): $(PCH_HEADER)
	@echo "Precompiling header..."
	$(CXX) $(COMMON_CXXFLAGS) $(shell $(WX_CONF_PATH) --cxxflags) -x c++-header -c $< -o $@

# --- Release Build ---
release: BIN_DIR  := $(BIN_DIR_ROOT)/release
release: OBJ_DIR  := $(OBJ_DIR_ROOT)/release
release: WX_CONF_PATH := $(WX_BASE_DIR)/build-gtk-release-shared_x64/wx-config
release: CXXFLAGS := $(COMMON_CXXFLAGS) -O3 -D_MPTRACE_
release: LDFLAGS  := -Wl,-rpath,'$$ORIGIN' $(SYS_LIBS)
release: $(PCH_GCH)
	@$(MAKE) _build BIN_DIR="$(BIN_DIR)" OBJ_DIR="$(OBJ_DIR)" WX_CONF_PATH="$(WX_CONF_PATH)" CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)"
	@echo "Copying required shared libraries..."
	@LD_LIBRARY_PATH=$(WX_BASE_DIR)/build-gtk-release-shared_x64/lib ldd $(BIN_DIR)/$(TARGET_NAME) | awk '/=> \.\//{print $$3}' | xargs -I {} cp -L {} $(BIN_DIR)/
	@strip --strip-unneeded $(BIN_DIR)/$(TARGET_NAME)
	@echo Copying config.xml...
	@cp -f "$(BASE_DIR)/config.xml" "$(BIN_DIR)/"
	@echo Copying scripts...
	@mkdir -p "$(BIN_DIR)/scripts"
	@cp -f "$(BASE_DIR)/scripts/"* "$(BIN_DIR)/scripts/"

# --- Run Phonies ---
rund:
	@$(MAKE) debug
	@LD_LIBRARY_PATH=$(WX_BASE_DIR)/build-gtk-debug-shared_x64/lib $(BIN_DIR_ROOT)/debug/$(TARGET_NAME)_debug

runr:
	@$(MAKE) release
	@$(BIN_DIR_ROOT)/release/$(TARGET_NAME)

runddd:
	@$(MAKE) debug
	@LD_LIBRARY_PATH=$(WX_BASE_DIR)/build-gtk-debug-shared_x64/lib ddd $(BIN_DIR_ROOT)/debug/$(TARGET_NAME)_debug &

# --- Internal Build Logic ---
_build:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	$(eval CURRENT_OBJECTS := $(patsubst $(SRC_DIR)/%.cxx, $(OBJ_DIR)/%.o, $(SOURCES)))
	@$(MAKE) .inner_link OBJECTS="$(CURRENT_OBJECTS)"

.inner_link: $(OBJECTS)
	$(eval OUT_NAME := $(if $(findstring debug,$(BIN_DIR)),$(TARGET_NAME)_debug,$(TARGET_NAME)))
	@echo "Linking $(OUT_NAME)..."
	$(CXX) $(OBJECTS) $(shell $(WX_CONF_PATH) --libs std,net,xml,stc,richtext) $(LDFLAGS) -o $(BIN_DIR)/$(OUT_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(shell $(WX_CONF_PATH) --cxxflags) -c $< -o $@

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR_ROOT) $(BIN_DIR_ROOT) $(PCH_GCH)
