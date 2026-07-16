#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <string>
#include "jsapi.h"  

/* RAII wrapper for managing SpiderMonkey engine lifecycle. Ensures the engine initializes and shuts down automatically,
preventing memory leaks during unexpected execution failures.
*/
class SpiderMonkeyScope {
public:
    SpiderMonkeyScope();
    ~SpiderMonkeyScope();

    JSContext* getContext() const { return cx; }
    bool isValid() const { return initialized && cx != nullptr; }

private:
    JSContext* cx;
    bool initialized;
};

// Utility function to read the entire file content into a native C++ string.
bool cpp_readFile(const std::string& filePath, std::string& output);

#endif
