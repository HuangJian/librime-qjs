#ifndef RIME_QJS_ENVIRONMENT_H_
#define RIME_QJS_ENVIRONMENT_H_

#include <rime/engine.h>
#include "quickjs.h"
#include "jsvalue_raii.h"
#include "qjs_helper.h"
#include "qjs_engine.h"

#include <string>
#include <filesystem>

namespace rime {

class QjsEnvironment {
public:
  // Create and initialize a JavaScript environment object
  static JSValueRAII Create(JSContext* ctx, Engine* engine, const std::string& name_space);

  // Add utility functions to the environment
  static void AddUtilityFunctions(JSContext* ctx, JSValue environment);

  // Helper function to call init function from a module namespace
  static bool CallInitFunction(JSContext* ctx, JSValue moduleNamespace, JSValue environment);

  // Helper function to call finit function
  static bool CallFinitFunction(JSContext* ctx, JSValue finitFunc, JSValue environment);

private:
  // Utility function to load file content
  static JSValue loadFile(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

  // Utility function to check if file exists
  static JSValue fileExists(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
};

} // namespace rime

#endif  // RIME_QJS_ENVIRONMENT_H_