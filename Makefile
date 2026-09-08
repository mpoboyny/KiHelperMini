include common.mk

BASE_DIR     := .
WX_BASE_DIR  := ./wxWidgets/3.3.1
SRC_DIR      := $(BASE_DIR)/source
INC_DIR      := $(BASE_DIR)/include
OBJ_DIR_ROOT := $(BASE_DIR)/obj
BIN_DIR_ROOT := $(BASE_DIR)/bin
LLAMA_BASE   := $(BASE_DIR)/llama.cpp
LLAMA_INC    := $(LLAMA_BASE)/llama.cpp-0.3.0/include
GGML_INC     := $(LLAMA_BASE)/llama.cpp-0.3.0/ggml/include

# --- Konfiguration ---
TARGET_NAME := KiHelperMini
SETUP_ARCHIVE_NAME := $(TARGET_NAME)_DebianTrixie_x64.7z

# Llama Shared Libs
LLAMA_LIBS := -lllama -lggml -lggml-base -lggml-cpu

# System Libs
SYS_LIBS   := -lpthread -lm -ldl -lrt -lX11 -lgomp

# --- Compiler Settings ---
# -fopenmp ist zwingend für die GOMP-Symbole
CURRENT_CXXFLAGS := $(COMMON_CXXFLAGS) -fopenmp -I$(INC_DIR) -D$(OS_DEF)
CURRENT_CXXFLAGS += -I$(LLAMA_INC) -I$(GGML_INC)

# --- PCH Settings ---
PCH_HEADER   := $(INC_DIR)/prc.hxx
PCH_GCH      := $(PCH_HEADER).gch

SOURCES := $(wildcard $(SRC_DIR)/*.cxx)

.PHONY: all debug release setup clean rund runr runddd echo _build .inner_link

all: release

echo:
	@echo "Available phonies:"
	@echo "  all debug release setup clean rund runr runddd echo"
	@echo
	@echo "Variables:"
	@echo "  BASE_DIR=$(BASE_DIR)"
	@echo "  WX_BASE_DIR=$(WX_BASE_DIR)"
	@echo "  SRC_DIR=$(SRC_DIR)"
	@echo "  INC_DIR=$(INC_DIR)"
	@echo "  OBJ_DIR_ROOT=$(OBJ_DIR_ROOT)"
	@echo "  BIN_DIR_ROOT=$(BIN_DIR_ROOT)"
	@echo "  LLAMA_BASE=$(LLAMA_BASE)"
	@echo "  LLAMA_INC=$(LLAMA_INC)"
	@echo "  GGML_INC=$(GGML_INC)"
	@echo "  LLAMA_LIBS=$(LLAMA_LIBS)"
	@echo "  TARGET_NAME=$(TARGET_NAME)"
	@echo "  OS_DEF=$(OS_DEF)"
	@echo "  CXX=$(CXX)"
	@echo "  CURRENT_CXXFLAGS=$(CURRENT_CXXFLAGS)"
	@echo "  SYS_LIBS=$(SYS_LIBS)"
	@echo "  PCH_HEADER=$(PCH_HEADER)"
	@echo "  PCH_GCH=$(PCH_GCH)"
	@echo "  SETUP_ARCHIVE_NAME=$(SETUP_ARCHIVE_NAME)"

$(PCH_GCH): $(PCH_HEADER)
	@echo "Precompiling header..."
	$(CXX) $(CURRENT_CXXFLAGS) $(shell $(WX_CONF_PATH) --cxxflags) -x c++-header -c $< -o $@

# --- Release Build ---
release: BIN_DIR  := $(BIN_DIR_ROOT)/release
release: OBJ_DIR  := $(OBJ_DIR_ROOT)/release
release: WX_CONF_PATH := $(WX_BASE_DIR)/build-gtk-release-shared_x64/wx-config
release: CXXFLAGS := $(CURRENT_CXXFLAGS) -O3 -D_MPTRACE_
release: LLAMA_LIB_DIR := $(LLAMA_BASE)/release-shared_x64-ncuda/lib
release: LDFLAGS  := -Wl,-rpath,'$$ORIGIN' -L$(LLAMA_LIB_DIR) $(LLAMA_LIBS) $(SYS_LIBS)
release: $(PCH_GCH)
	@$(MAKE) _build BIN_DIR="$(BIN_DIR)" OBJ_DIR="$(OBJ_DIR)" WX_CONF_PATH="$(WX_CONF_PATH)" CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)"
	@echo "Copying required shared libraries..."
	@LD_LIBRARY_PATH=$(LLAMA_LIB_DIR):$(WX_BASE_DIR)/build-gtk-release-shared_x64/lib ldd $(BIN_DIR)/$(TARGET_NAME) | awk '/=> \.\//{print $$3}' | xargs -I {} cp -L {} $(BIN_DIR)/
	@strip --strip-unneeded $(BIN_DIR)/$(TARGET_NAME)
	@echo Copying config.xml...
	@cp -f "$(BASE_DIR)/config.xml" "$(BIN_DIR)/"
	@echo Copying scripts...
	@mkdir -p "$(BIN_DIR)/scripts"
	@cp -f "$(BASE_DIR)/scripts/"* "$(BIN_DIR)/scripts/"
	@echo Copying ini...
	@mkdir -p "$(BIN_DIR)/ini"
	@cp -f "$(BASE_DIR)/ini/"* "$(BIN_DIR)/ini/"
	@echo Copying tools...
	@mkdir -p "$(BIN_DIR)/tools"
	@cp -f "$(BASE_DIR)/tools/"* "$(BIN_DIR)/tools/"

setup: release
	@echo "Creating 7z archive..."
	@rm -fr "$(BIN_DIR_ROOT)/setup"
	@mkdir -p "$(BIN_DIR_ROOT)/setup"
	@mkdir -p "$(BIN_DIR_ROOT)/setup/$(TARGET_NAME)"
	@cp -fr "$(BIN_DIR_ROOT)/release/" "$(BIN_DIR_ROOT)/setup/$(TARGET_NAME)/"
	@7z a -t7z -mx=9 "$(BIN_DIR_ROOT)/setup/$(SETUP_ARCHIVE_NAME)" "$(BIN_DIR_ROOT)/setup/$(TARGET_NAME)"
	
# --- Run Phonies ---
rund:
	@$(MAKE) debug
	@LD_LIBRARY_PATH=$(WX_BASE_DIR)/build-gtk-debug-shared_x64/lib $(BIN_DIR_ROOT)/debug/$(TARGET_NAME)_debug

runr:
	@$(MAKE) release
	@LD_LIBRARY_PATH=$(BIN_DIR_ROOT)/release:$(WX_BASE_DIR)/build-gtk-release-shared_x64/lib $(BIN_DIR_ROOT)/release/$(TARGET_NAME)

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
