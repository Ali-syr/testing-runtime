#include <iostream>
#include "core/engine.hpp"
#include "bindings/native.hpp"
#include "js/Initialization.h"
#include "js/CompilationAndEvaluation.h"
#include "js/SourceText.h"

int main(int argc, char* argv[]) {
    // Validate command-line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.js>" << std::endl;
        return 1;
    }

    std::string scriptPath = argv[1];
    std::string jsCode;
    
    // Load target JavaScript file into host memory
    if (!cpp_readFile(scriptPath, jsCode)) {
        std::cerr << "Error: Could not read file or file is empty: " << scriptPath << std::endl;
        return 1;
    }

    std::cout << "Starting SpiderMonkey initialization" << std::endl;

    // Initialize engine lifecycle using the RAII class wrapper
    SpiderMonkeyScope engineScope;
    if (!engineScope.isValid()) {
        std::cerr << "Error: Failed to initialize SpiderMonkey runtime!" << std::endl;
        return 1;
    }
    JSContext* cx = engineScope.getContext();

    // Initialize SpiderMonkey internal self-hosted JS features
    if (!JS::InitSelfHostedCode(cx)) {
        std::cerr << "Error: Failed to InitSelfHostedCode!" << std::endl;
        return 1;
    }

    {
        // Define configuration for the global scope environment object
        // FIXED: Used C++20 designated initializers to safely clear -Wmissing-field-initializers warning
        static const JSClass globalClass = {
            .name = "global",
            .flags = JSCLASS_GLOBAL_FLAGS,
            .cOps = &JS::DefaultGlobalClassOps
        };

        JS::RealmOptions options;
        // Instantiate the root global object (acts as 'globalThis')
        JS::RootedObject global(cx, JS_NewGlobalObject(cx, &globalClass, nullptr, JS::DontFireOnNewGlobalHook, options));
        if (!global) return 1;

        // Enter the execution Realm boundary of the global object
        JSAutoRealm arRealm(cx, global);

        // Inject custom native C++ functions into the global JS scope
        if (!JS_DefineFunctions(cx, global, global_functions)) return 1;

        // Map 'globalThis' property keyword to point back to the global scope object itself
        if (!JS_DefineProperty(cx, global, "globalThis", global, JSPROP_READONLY | JSPROP_PERMANENT)) return 1;

        // Set compilation metadata for stack traces and error catching
        JS::CompileOptions compileOptions(cx);
        compileOptions.setFileAndLine(scriptPath.c_str(), 1);

        // Pack the raw JS string into SpiderMonkey's Utf8Unit SourceText container
        JS::SourceText<mozilla::Utf8Unit> source;
        if (!source.init(cx, jsCode.c_str(), jsCode.length(), JS::SourceOwnership::Borrowed)) return 1;

        std::cout << "Executing " << scriptPath << "..." << std::endl;
        
        // Compile and evaluate the wrapped JavaScript code block
        JS::RootedValue rval(cx);
        if (!JS::Evaluate(cx, compileOptions, source, &rval)) {
            std::cerr << "Runtime Error: JavaScript execution failed!" << std::endl;
            
            // Extract error trace details if a pending JS exception exists
            JS::ExceptionStack exnStack(cx);
            if (JS::StealPendingExceptionStack(cx, &exnStack)) {
                JS::ErrorReportBuilder report(cx);
                if (report.init(cx, exnStack, JS::ErrorReportBuilder::WithSideEffects)) {
                    std::cerr << "JS Error Details:\n" << report.toStringResult().c_str() << std::endl;
                }
            }
        } else {
            std::cout << "JavaScript execution finished successfully!" << std::endl;
        }
    }

    return 0;
}
