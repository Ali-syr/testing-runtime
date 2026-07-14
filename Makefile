# Default to the local symlink created
SPIDERMONKEY_DIR ?= ./spidermonkey

CXX = g++
CXXFLAGS = -O3 -DDEBUG -DXP_LINUX -DXP_UNIX -DJS_STANDALONE -I$(SPIDERMONKEY_DIR)/dist/include
LDFLAGS = -L$(SPIDERMONKEY_DIR)/dist/bin -lmozjs-154a1 -Wl,-rpath,'$$ORIGIN/../spidermonkey/dist/bin'

# Target 
all: bin/testing

bin/testing: src/main.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) src/main.cpp -o bin/testing $(LDFLAGS)

# Helper Target running
run: all
	./bin/testing test.js

clean:
	rm -rf bin/
