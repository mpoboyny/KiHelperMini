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
GTEST_BASE       := $(HELPERCLI_DIR)../googletest
GTEST_INC_PATH   := $(GTEST_BASE)/googletest/include
GTEST_LIB_PATH   := $(GTEST_BASE)/release-shared_x64/lib

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
SOURCES := source/pre.cxx source/cli.cxx source/cli-client.cxx source/cli-context.cxx source/AudioHandler.cxx source/ClearHandler.cxx source/ExitHandler.cxx source/GlobHandler.cxx source/ImageHandler.cxx source/ReadHandler.cxx source/RegenerateHandler.cxx source/RunHandler.cxx source/VideoHandler.cxx source/main.cxx
OBJ_FILES := $(patsubst source/%.cxx,$(OBJ_DIR)/%.o,$(SOURCES))
HEADERS := include/cli-client.hxx include/cli-context.hxx include/cli-server.hxx include/cli-ui.hxx include/cli-todo.hxx include/cli-text.hxx include/AudioHandler.hxx include/ClearHandler.hxx include/ExitHandler.hxx include/GlobHandler.hxx include/ImageHandler.hxx include/ReadHandler.hxx include/RegenerateHandler.hxx include/RunHandler.hxx include/VideoHandler.hxx
TEST_BIN_DIR := $(BIN_DIR_ROOT)/tests/$(BUILD_TYPE)
TEST_OBJ_DIR := $(OBJ_DIR_ROOT)/tests/$(BUILD_TYPE)
TEST_TARGET := $(TEST_BIN_DIR)/helperCli-tests
TEST_SOURCES := $(wildcard tests/*.cxx)
TEST_OBJ_FILES := $(patsubst tests/%.cxx,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))
SCRIPTS := $(wildcard scripts/*)

.NOTPARALLEL:
.PHONY: all release test sim1 sim2 clean

all: release

release: $(TARGET) $(TEST_TARGET)

test: $(TARGET) $(TEST_TARGET)
	$(TEST_TARGET)

sim1: release
	$(MAKE) -C simulation/RunAndExit release
	$(HELPERCLI_DIR)bin/simulation/RunAndExit/release/RunAndExit

sim2: release
	$(MAKE) -C simulation/RunDirExit release
	$(HELPERCLI_DIR)bin/simulation/RunDirExit/release/RunDirExit

$(TARGET): $(OBJ_FILES) $(SCRIPTS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking $@..."
	$(CXX) $(OBJ_FILES) $(LDFLAGS) $(LLAMA_LIBS) $(COMMON_LIBS) $(SERVER_LIBS) $(HTTPLIB_LIBS) $(SYS_LIBS) -o $@
	@test -x $@
	@echo "Copying required llama.cpp shared libraries..."
	@for library in $(LLAMA_LIB_PATH)/libllama.so* $(LLAMA_LIB_PATH)/libggml.so* $(LLAMA_LIB_PATH)/libggml-base.so* $(LLAMA_LIB_PATH)/libggml-cpu.so* $(LLAMA_LIB_PATH)/libggml-cuda.so* $(LLAMA_LIB_PATH)/libllama-common.so* $(LLAMA_LIB_PATH)/libllama-server-impl.so* $(LLAMA_LIB_PATH)/libmtmd.so*; do \
		[ -e "$$library" ] && cp -a "$$library" $(BIN_DIR)/; \
	done
	@echo "Copying scripts..."
	@rm -rf $(BIN_DIR)/scripts
	@cp -a scripts $(BIN_DIR)/
	@test -x $@
	@echo "Release binary: $@"

$(OBJ_DIR)/%.o: source/%.cxx $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJ_FILES) tests
	@mkdir -p $(TEST_BIN_DIR)
	$(CXX) $(TEST_OBJ_FILES) -L$(GTEST_LIB_PATH) -Wl,-rpath,'$$ORIGIN' -lgtest -pthread -o $@
	@cp -a $(GTEST_LIB_PATH)/libgtest.so* $(TEST_BIN_DIR)/

$(TEST_OBJ_DIR)/%.o: tests/%.cxx tests/common.hxx include/cli-todo.hxx include/cli-text.hxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(GTEST_INC_PATH) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR_ROOT)
	@mkdir -p $(BIN_DIR)
	@find $(BIN_DIR) -mindepth 1 -maxdepth 1 -exec rm -rf {} +
	@rm -rf $(TEST_BIN_DIR)
