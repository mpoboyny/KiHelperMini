include ../common.mk

CXX ?= g++

HELPERCLI_DIR    := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))/
LLAMA_BASE       := $(HELPERCLI_DIR)../llama.cpp
LLAMA_INC_PATH   := $(LLAMA_BASE)/include
GGML_INC_PATH    := $(LLAMA_BASE)/ggml/include
COMMON_INC_PATH  := $(LLAMA_BASE)/common
HTTPLIB_INC_PATH := $(LLAMA_BASE)/vendor
KIHELPER_INC_PATH := $(HELPERCLI_DIR)../include
LLAMA_LIB_PATH   := $(LLAMA_BASE)/release-shared_x64-cuda/lib

# CXXFLAGS := $(COMMON_CXXFLAGS) -O3 -DNDEBUG -D_MPTRACE_ -DLLAMA_SHARED -DGGML_SHARED
CXXFLAGS := $(COMMON_CXXFLAGS) -O3 -DNDEBUG -DLLAMA_SHARED -DGGML_SHARED
CXXFLAGS += -DCPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH=1048576 -DCPPHTTPLIB_LISTEN_BACKLOG=512 -DCPPHTTPLIB_OPENSSL_SUPPORT -DCPPHTTPLIB_REQUEST_URI_MAX_LENGTH=32768 -DCPPHTTPLIB_TCP_NODELAY=1
CXXFLAGS += -Iinclude -I$(LLAMA_INC_PATH) -I$(GGML_INC_PATH) -I$(COMMON_INC_PATH) -I$(HTTPLIB_INC_PATH) -I$(KIHELPER_INC_PATH)

LDFLAGS := -L$(LLAMA_LIB_PATH) -Wl,-rpath,'$$ORIGIN'
LLAMA_LIBS := -lllama -lggml -lggml-base -lggml-cpu
COMMON_LIBS := -lllama-common
SERVER_LIBS := -lllama-server-impl
HTTPLIB_LIBS := -lcpp-httplib
SYS_LIBS := -pthread -lm -ldl -lrt -lgomp -lssl -lcrypto

OBJ_DIR_ROOT := $(HELPERCLI_DIR)obj
BIN_DIR_ROOT := $(HELPERCLI_DIR)bin
BUILD_TYPE := release
OBJ_DIR := $(OBJ_DIR_ROOT)/$(BUILD_TYPE)
BIN_DIR := $(BIN_DIR_ROOT)/$(BUILD_TYPE)

TARGET := $(BIN_DIR)/helperCli
SOURCES := source/pre.cxx source/cli.cxx source/cli-client.cxx source/cli-context.cxx source/RunHandler.cxx source/main.cxx
OBJ_FILES := $(patsubst source/%.cxx,$(OBJ_DIR)/%.o,$(SOURCES))
HEADERS := include/cli-client.hxx include/cli-context.hxx include/cli-server.hxx include/cli-ui.hxx include/RunHandler.hxx

.NOTPARALLEL:
.PHONY: all release clean

all: release

release: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	@echo "Linking $@..."
	$(CXX) $(OBJ_FILES) $(LDFLAGS) $(LLAMA_LIBS) $(COMMON_LIBS) $(SERVER_LIBS) $(HTTPLIB_LIBS) $(SYS_LIBS) -o $@
	@test -x $@
	@echo "Copying required llama.cpp shared libraries..."
	@for library in $(LLAMA_LIB_PATH)/libllama.so* $(LLAMA_LIB_PATH)/libggml.so* $(LLAMA_LIB_PATH)/libggml-base.so* $(LLAMA_LIB_PATH)/libggml-cpu.so* $(LLAMA_LIB_PATH)/libggml-cuda.so* $(LLAMA_LIB_PATH)/libllama-common.so* $(LLAMA_LIB_PATH)/libllama-server-impl.so* $(LLAMA_LIB_PATH)/libmtmd.so*; do \
		[ -e "$$library" ] && cp -a "$$library" $(BIN_DIR)/; \
	done
	@test -x $@
	@echo "Release binary: $@"

$(OBJ_DIR)/%.o: source/%.cxx $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR_ROOT)
	@mkdir -p $(BIN_DIR)
	@find $(BIN_DIR) -mindepth 1 -maxdepth 1 -exec rm -rf {} +
