include common.mk

BASE_DIR     := .
WX_BASE_DIR  := ./wxWidgets/3.3.1
SRC_DIR      := $(BASE_DIR)/source
INC_DIR      := $(BASE_DIR)/include
OBJ_DIR_ROOT := $(BASE_DIR)/obj
BIN_DIR_ROOT := $(BASE_DIR)/bin
LLAMA_BASE   := $(BASE_DIR)/llama.cpp
LLAMA_INC    := $(LLAMA_BASE)/include
GGML_INC     := $(LLAMA_BASE)/ggml/include

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

.PHONY: all release build_safe_ldd _build .inner_link runr echo _build .inner_link

all: build_safe_ldd
release: build_safe_ldd

$(PCH_GCH): $(PCH_HEADER)
	@echo "Precompiling header..."
	$(CXX) $(CURRENT_CXXFLAGS) $(shell $(WX_CONF_PATH) --cxxflags) -x c++-header -c $< -o $@

# --- Release Build ---
release: BIN_DIR  := $(BIN_DIR_ROOT)/release
release: OBJ_DIR  := $(OBJ_DIR_ROOT)/release
release: WX_CONF_PATH := $(WX_BASE_DIR)/build-gtk-release-shared_x64/wx-config
release: CXXFLAGS := $(CURRENT_CXXFLAGS) -O3 -D_MPTRACE_
release: LLAMA_LIB_DIR := $(LLAMA_BASE)/release-shared_x64-cuda/lib
release: LDFLAGS  := -L$(LLAMA_LIB_DIR) $(LLAMA_LIBS) $(SYS_LIBS)
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
	@rm -rf "$(BIN_DIR)/tools/safe_ldd"
	@mkdir -p "$(BIN_DIR)/tools/safe_ldd"
	@cp -f "$(BIN_DIR_ROOT)/release/tools_staging/safe_ldd" "$(BIN_DIR)/tools/safe_ldd/safe_ldd"
	@cp -f "$(BASE_DIR)/safe_ldd/README.md" "$(BIN_DIR)/tools/safe_ldd/"
	@cp -f "$(BASE_DIR)/safe_ldd/KiHelperMiniRunner.sh" "$(BIN_DIR)/tools/safe_ldd/"

build_safe_ldd:
	@$(MAKE) -C safe_ldd all

setup: release
	@echo "Creating 7z archive..."
	@rm -fr "$(BIN_DIR_ROOT)/setup"
	@mkdir -p "$(BIN_DIR_ROOT)/setup"
	@mkdir -p "$(BIN_DIR_ROOT)/setup/$(TARGET_NAME)"
	@cp -fr "$(BIN_DIR_ROOT)/release/." "$(BIN_DIR_ROOT)/setup/$(TARGET_NAME)/"
	@cd "$(BIN_DIR_ROOT)/setup" && 7z a -t7z -mx=9 "$(SETUP_ARCHIVE_NAME)" "$(TARGET_NAME)"
	
# --- Internal Build Logic ---
_build:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	$(eval CURRENT_OBJECTS := $(patsubst $(SRC_DIR)/%.cxx, $(OBJ_DIR)/%.o, $(SOURCES)))
	@$(MAKE) .inner_link OBJECTS="$(CURRENT_OBJECTS)"

.inner_link: $(OBJECTS)
	$(eval OUT_NAME := $(if $(findstring debug,$(BIN_DIR)),$(TARGET_NAME)_debug,$(TARGET_NAME)))
	@echo "Linking $(OUT_NAME)..."
	$(CXX) $(OBJECTS) $(shell $(WX_CONF_PATH) --libs std,net,xml,stc,richtext) $(LDFLAGS) -Wl,-rpath,'$$ORIGIN' -o $(BIN_DIR)/$(OUT_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(shell $(WX_CONF_PATH) --cxxflags) -c $< -o $@

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR_ROOT) $(BIN_DIR_ROOT) $(PCH_GCH)
	@rm -rf $(BIN_DIR_ROOT)/tools/safe_ldd
