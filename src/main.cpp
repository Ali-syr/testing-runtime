#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "jsapi.h"
#include "js/Initialization.h"
#include "js/Context.h"
#include "js/CompilationAndEvaluation.h"
#include "js/SourceText.h"
#include "js/Conversions.h"

// Helper function to read the entire file content into a string
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// --- NEW: C++ Implementation of the JavaScript print() function ---
bool js_print(JSContext* cx, unsigned argc, JS::Value* vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    
    // Loop through all arguments passed to print(a, b, c, ...)
    for (unsigned i = 0; i < args.length(); i++) {
        JS::RootedString str(cx, JS::ToString(cx, args[i]));
        if (!str) {
            return false;
        }
        
        // Convert SpiderMonkey string to standard C++ string
        JS::UniqueChars chars = JS_EncodeStringToUTF8(cx, str);
        if (!chars) {
            return false;
        }
        
        std::cout << chars.get() << (i == args.length() - 1 ? "" : " ");
    }
    
    std::cout << std::endl; // Append newline at the end
    args.rval().setUndefined(); // Return undefined back to JS
    return true;
}

// Define the native functions to be linked into the Global Object
static const JSFunctionSpec global_functions[] = {
    JS_FN("print", js_print, 1, 0),
    JS_FS_END
};
// ------------------------------------------------------------------

int main(int argc, char* argv[]) {
    // Validate command-line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.js>" << std::endl;
        return 1;
    }

    std::string scriptPath = argv[1];
    std::string jsCode = readFile(scriptPath);
    if (jsCode.empty()) {
        std::cerr << "Error: Could not read file or file is empty: " << scriptPath << std::endl;
        return 1;
    }

    std::cout << "Starting SpiderMonkey initialization..." << std::endl;

    // 1. Initialize the global JS engine state.
    if (!JS_Init()) {
        std::cerr << "Error: Failed to initialize global JS_Init!" << std::endl;
        return 1;
    }

    // 2. Create the JS Context.
    JSContext* cx = JS_NewContext(JS::DefaultHeapMaxBytes);
    if (!cx) {
        std::cerr << "Error: Failed to create JSContext!" << std::endl;
        JS_ShutDown();
        return 1;
    }

    // 3. Initialize Self-Hosted Code.
    if (!JS::InitSelfHostedCode(cx)) {
        std::cerr << "Error: Failed to InitSelfHostedCode!" << std::endl;
        JS_DestroyContext(cx);
        JS_ShutDown();
        return 1;
    }

    // --- JAVASCRIPT EXECUTION STAGE ---
    {
        // Define the standard global class configuration
        static JSClass globalClass = {
            "global",
            JSCLASS_GLOBAL_FLAGS,
            &JS::DefaultGlobalClassOps
        };

        JS::RealmOptions options;
        
        // Create the global object (acts as 'globalThis' or 'window')
        JS::RootedObject global(cx, JS_NewGlobalObject(cx, &globalClass, nullptr, JS::DontFireOnNewGlobalHook, options));
        if (!global) {
            std::cerr << "Error: Failed to create global object!" << std::endl;
            JS_DestroyContext(cx);
            JS_ShutDown();
            return 1;
        }

        // Enter the Realm/execution context of the newly created global object
        JSAutoRealm arRealm(cx, global);

        // --- NEW: Inject the native C++ print function into the JS global scope ---
        if (!JS_DefineFunctions(cx, global, global_functions)) {
            std::cerr << "Error: Failed to define global functions!" << std::endl;
            JS_DestroyContext(cx);
            JS_ShutDown();
            return 1;
        }
        // -------------------------------------------------------------------------

        // Set compilation options (stores filename for stack traces and error reporting)
        JS::CompileOptions compileOptions(cx);
        compileOptions.setFileAndLine(scriptPath.c_str(), 1);

        // Wrap the JS source code string inside SpiderMonkey's SourceText container
        JS::SourceText<mozilla::Utf8Unit> source;
        if (!source.init(cx, jsCode.c_str(), jsCode.length(), JS::SourceOwnership::Borrowed)) {
            std::cerr << "Error: Failed to initialize SourceText!" << std::endl;
            JS_DestroyContext(cx);
            JS_ShutDown();
            return 1;
        }

        std::cout << "Executing " << scriptPath << "..." << std::endl;
        
        // Compile and evaluate the JavaScript source code
        JS::RootedValue rval(cx);
        if (!JS::Evaluate(cx, compileOptions, source, &rval)) {
            std::cerr << "Runtime Error: JavaScript execution failed!" << std::endl;
            
            // Catch and print the actual JS exception
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
    // --- END OF EXECUTION STAGE ---

    // 4. Perform proper cleanup in reverse order of initialization.
    JS_DestroyContext(cx);
    JS_ShutDown();
    
    return 0;
}
