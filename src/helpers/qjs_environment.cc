#include <rime_api.h>
#include <quickjs.h>
#include <cstddef>
#include <sstream>
#include <cstdio>
#include <iostream>

#include "qjs_environment.h"
#include "qjs_helper.h"
#include "qjs_engine.h"
#include "jsstring_raii.h"
#include "process_memory.h"

namespace rime {

JSValueRAII QjsEnvironment::create(JSContext* ctx, Engine* engine, const std::string& nameSpace) {
  JSValueRAII environment(JS_NewObject(ctx)); // do not free its properties/methods manually
  JS_SetPropertyStr(ctx, environment, "engine", QjsEngine::Wrap(ctx, engine));
  JS_SetPropertyStr(ctx, environment, "namespace", JS_NewString(ctx, nameSpace.c_str()));
  JS_SetPropertyStr(ctx, environment, "userDataDir", JS_NewString(ctx, QjsHelper::basePath.c_str()));

  // Add utility functions
  addUtilityFunctions(ctx, environment);

  return environment;
}

void QjsEnvironment::addUtilityFunctions(JSContext* ctx, JSValue environment) {
  JS_SetPropertyStr(ctx, environment, "loadFile", JS_NewCFunction(ctx, loadFile, "loadFile", 1));
  JS_SetPropertyStr(ctx, environment, "fileExists", JS_NewCFunction(ctx, fileExists, "fileExists", 1));
  JS_SetPropertyStr(ctx, environment, "getRimeInfo", JS_NewCFunction(ctx, getRimeInfo, "getRimeInfo", 0));
  JS_SetPropertyStr(ctx, environment, "popen", JS_NewCFunction(ctx, popen, "popen", 1));
}

JSValue QjsEnvironment::loadFile(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (path == nullptr) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  std::string content = QjsHelper::loadFile(path);
  JS_FreeCString(ctx, path);

  return JS_NewString(ctx, content.c_str());
}

static std::string formatMemoryUsage(size_t usage) {
  constexpr size_t KILOBYTE = 1024;
  return usage > KILOBYTE * KILOBYTE
    ? std::to_string(usage / KILOBYTE / KILOBYTE) + "M" // in MB
    : std::to_string(usage / KILOBYTE) + "K"; // in KB
}

JSValue QjsEnvironment::getRimeInfo(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  size_t vmUsage = 0;
  size_t residentSet = 0; // memory usage in bytes
  getMemoryUsage(vmUsage, residentSet);

  JSMemoryUsage qjsMemStats;
  JS_ComputeMemoryUsage(JS_GetRuntime(ctx), &qjsMemStats);

  std::stringstream ss;
  ss << "libRime v" << rime_get_api()->get_version() << " | "
     << "libRime-qjs v" << RIME_QJS_VERSION << " | "
     << "Process RSS Mem: " << formatMemoryUsage(residentSet) << " | "
     << "QuickJS Mem: " << formatMemoryUsage(qjsMemStats.memory_used_size);

  return JS_NewString(ctx, ss.str().c_str());
}

JSValue QjsEnvironment::fileExists(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument is required");
  }

  const char* path = JS_ToCString(ctx, argv[0]);
  if (path == nullptr) {
    return JS_ThrowSyntaxError(ctx, "The absolutePath argument should be a string");
  }

  bool exists = std::filesystem::exists(path);
  JS_FreeCString(ctx, path);

  return JS_NewBool(ctx, exists);
}

JSValue QjsEnvironment::popen(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "The command argument is required");
  }

  auto command = JSStringRAII(JS_ToCString(ctx, argv[0]));
  if (command.cStr() == nullptr) {
    return JS_ThrowSyntaxError(ctx, "The command argument should be a string");
  }

  // Open a pipe to the command
  FILE* pipe = ::popen(command.cStr(), "r");
  if (pipe == nullptr) {
    LOG(ERROR) << "Failed to run command: " << command.cStr();
    return JS_ThrowPlainError(ctx, "Failed to run command %s", command.cStr());
  }

  // Read the output
  constexpr size_t READ_BUFFER_SIZE = 128;
  char buffer[READ_BUFFER_SIZE];
  std::string result;
  while (fgets(static_cast<char*>(buffer), sizeof(buffer), pipe) != nullptr) {
      result += static_cast<char*>(buffer);
  }

  // Close the pipe
  int status = pclose(pipe);
  if (status == 0) {
    DLOG(INFO) << "Command output: " << result;
  } else {
    LOG(ERROR) << "Command failed with status: " << status;
    return JS_ThrowPlainError(ctx, "Command failed with status: %d", status);
  }
  return JS_NewString(ctx, result.c_str());
}

} // namespace rime
