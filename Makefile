# Path to the local symlink of the SpiderMonkey standalone build directory
SPIDERMONKEY_DIR ?= ./spidermonkey

# Compiler settings
CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -DDEBUG -DXP_LINUX -DXP_UNIX -DJS_STANDALONE -I$(SPIDERMONKEY_DIR)/dist/include

# Linker settings (links libmozjs and sets relative rpath for shared library discovery)
LDFLAGS = -L$(SPIDERMONKEY_DIR)/dist/bin -lmozjs-154a1 -Wl,-rpath,'$$ORIGIN/../spidermonkey/dist/bin'

# All source files required to build the runtime binary
SOURCE = src/cli.cpp src/core/engine.cpp src/bindings/native.cpp

# Target binary path
TARGET = bin/testing

# Default target rule
all: $(TARGET)

# Rule to compile the source files and build the target executable
$(TARGET): $(SOURCE)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

# Helper target to build and execute the local test script automatically
run: all
	./bin/testing test/test.js 

# Clean rule to wipe the compiled binary directory
clean:
	rm -rf bin/
