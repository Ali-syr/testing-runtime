#include "engine.hpp"
#include <iostream>
#include <fstream>
#include "js/Initialization.h"
#include "js/Context.h"

SpiderMonkeyScope::SpiderMonkeyScope() : cx(nullptr), initialized(false) {
    // Initialize the SpiderMonkey library internal structures
    if (JS_Init()) {
        initialized = true;
        cx = JS_NewContext(32 * 1024 * 1024); 
    }
}

SpiderMonkeyScope::~SpiderMonkeyScope() {
    // Safely destroy context and shutdown engine on scope destruction
    if (cx) {
        JS_DestroyContext(cx);
    }
    if (initialized) {
        JS_ShutDown();
    }
}

bool cpp_readFile(const std::string& filePath, std::string& output) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Determine file size to pre-allocate string memory efficiently
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    output.resize(size);

    // Read the file stream directly into the string container
    file.seekg(0, std::ios::beg);
    file.read(&output[0], size);
    return true;
}
