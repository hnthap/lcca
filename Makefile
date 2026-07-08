# ==============================================================================
#  Cross-Platform Wrapper Makefile for CMake
#  Works on Linux, macOS, and Windows (via MinGW Make or Git Bash)
# ==============================================================================

# --- Configuration ---
BUILD_DIR := build
GENERATOR := 
# Uncomment to force MinGW on Windows:
# GENERATOR = -G "MinGW Makefiles"

# --- Tools ---
CC=gcc
CMAKE_CMD := cmake
CTEST_CMD := ctest

# --- Default Flags ---
BUILD_TYPE ?= Debug
DEV_MODE   ?= ON

.PHONY: all build test clean release dev help docs clean_docs

# Default Target
all: build

# 1. Developer Mode (Default)
# Result: Debug symbols, Sanitizers ON, Warnings as Errors
dev: BUILD_TYPE := Debug
dev: DEV_MODE   := ON
dev: clean configure build

# 2. Release Build
# Result: Optimization -O3, Sanitizers OFF
release: BUILD_TYPE := Release
release: DEV_MODE   := OFF
release: clean configure build

# --- Core Steps ---

# 1. Configure

configure: $(BUILD_DIR)/CMakeLists.txt

$(BUILD_DIR)/CMakeLists.txt: CMakeLists.txt
	@echo " [Configure] Configured for $(BUILD_TYPE) (DevMode: $(DEV_MODE))"
	$(CMAKE_CMD) -E make_directory "$(BUILD_DIR)"
	$(CMAKE_CMD) -S . -B "$(BUILD_DIR)" \
		$(GENERATOR) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DLCCA_DEVELOPER_MODE=$(DEV_MODE)

# 2. Build

build: configure
	@echo " [Build] Building project..."
	$(CMAKE_CMD) --build "$(BUILD_DIR)"
	$(CMAKE_CMD) -E copy_if_different "$(BUILD_DIR)/compile_commands.json" .

# 3. Test

test: build
	@echo " [Test] Running suite..."
	$(CTEST_CMD) --test-dir "$(BUILD_DIR)" --output-on-failure

# 4. Clean Step
clean:
	@echo " [Clean] Removing build directory..."
	$(CMAKE_CMD) -E remove_directory "$(BUILD_DIR)"
	$(CMAKE_CMD) -E remove -f compile_commands.json

# Help

help:
	@echo "Available targets:"
	@echo "  make dev      : (Default) Rebuild in Debug mode (Sanitizers + Strict Warnings)"
	@echo "  make release  : Rebuild in Release mode (Optimized)"
	@echo "  make build    : Build the current configuration without cleaning"
	@echo "  make test     : Run the test suite"
	@echo "  make clean    : Remove build artifacts"

# --- Documentation settings ---
DOXYFILE := Doxyfile
DOC_DIR  := docs

# Generates the documentation
docs:
	@echo "Generating documentation..."
	doxygen $(DOXYFILE)
	@echo "Documentation generated in $(DOC_DIR)/html/index.html"

# Removes the generated files
clean_docs:
	rm -rf $(DOC_DIR)
