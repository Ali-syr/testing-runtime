#include "native.hpp"
#include "../core/engine.hpp"
#include <iostream>
#include "js/Conversions.h"

bool js_print(JSContext* cx, unsigned argc, JS::Value* vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    
    // Root the JS string once outside the loop to prevent repetitive GC rooting registration
    JS::RootedString str(cx);

    for (unsigned i = 0; i < args.length(); i++) {
        // Safely convert the argument to string and set it inside the rooted container
        str.set(JS::ToString(cx, args[i]));
        if (!str) return false;
        
        // Encode the SpiderMonkey string layout into a standard UTF-8 buffer
        JS::UniqueChars chars = JS_EncodeStringToUTF8(cx, str);
        if (!chars) return false;
        
        // Print and handle spacing between multiple arguments
        std::cout << chars.get() << (i == args.length() - 1 ? "" : " ");
    }
    
    std::cout << std::flush << '\n'; // Flush terminal buffer immediately
    args.rval().setUndefined();       // print() returns undefined in JS
    return true;
}

bool js_readFile(JSContext* cx, unsigned argc, JS::Value* vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    // Enforce argument validation
    if (args.length() < 1 || !args[0].isString()) {
        JS_ReportErrorUTF8(cx, "readFile() requires at least one string argument (filePath)");
        return false;
    }

    // Root the file path string before parsing to protect it from GC sweeps
    JS::RootedString str(cx, args[0].toString());
    JS::UniqueChars filePath = JS_EncodeStringToUTF8(cx, str);
    if (!filePath) return false;

    // Security Sandbox: basic path traversal prevention
    std::string pathStr(filePath.get());
    if (pathStr.find("..") != std::string::npos) {
        JS_ReportErrorUTF8(cx, "Access denied: Path traversal is prohibited ('..')");
        return false;
    }

    // Read the file content using the native core utility
    std::string content;
    if (!cpp_readFile(filePath.get(), content)) {
        JS_ReportErrorUTF8(cx, "Failed to open or read file: '%s'", filePath.get());
        return false;
    }

    // Convert the native string back into a GC-managed SpiderMonkey string
    JS::RootedString jsContent(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(content.c_str(), content.length())));
    if (!jsContent) return false;

    args.rval().setString(jsContent); // Pass the string value back to the JS context frame
    return true;
}

// functions to names accessible inside the JS environment
const JSFunctionSpec global_functions[] = {
    JS_FN("print", js_print, 1, 0),
    JS_FN("readFile", js_readFile, 1, 0),
    JS_FS_END
};
