#include "jsc_code_loader.h"
#include <glog/logging.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "engines/javascriptcore/jsc_string_raii.hpp"
#include "engines/jscode_utils.h"

std::pair<std::string, std::filesystem::path> JscCodeLoader::loadModuleSource(
    [[maybe_unused]] JSContextRef ctx,
    const std::string& baseFolderPath,
    const std::string& moduleName) {
  std::string possibleFileNames[] = {"dist/" + moduleName + ".iife.js",
                                     "dist/" + moduleName + ".js", moduleName + ".iife.js",
                                     moduleName + ".js"};
  for (const auto& fileName : possibleFileNames) {
    std::filesystem::path filePath = std::filesystem::path(baseFolderPath) / fileName;
    if (!std::filesystem::exists(filePath)) {
      continue;
    }

    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto source = buffer.str();
    removeExportStatementsInPlace(source);
    return {source, filePath};
  }

  return {"", std::filesystem::path()};
}

JSObjectRef JscCodeLoader::createInstanceOfIifeBundledModule(JSContextRef ctx,
                                                             const std::string& baseFolderPath,
                                                             const std::string& moduleName,
                                                             const std::vector<JSValueRef>& args,
                                                             JSValueRef* exception) {
  auto [source, filePath] = loadModuleSource(ctx, baseFolderPath, moduleName);
  if (source.empty()) {
    std::string message = "Failed to open file for module: " + moduleName;
    *exception = JSValueMakeString(ctx, JscStringRAII(message.c_str()));
    return nullptr;
  }

  std::string flatNamespace = moduleName;
  std::replace(flatNamespace.begin(), flatNamespace.end(), '.', '_');
  std::replace(flatNamespace.begin(), flatNamespace.end(), '-', '_');
  auto now = std::chrono::system_clock::now();
  auto tick = std::to_string(
      std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
  flatNamespace += "_" + tick;
  std::string instanceName = flatNamespace + "_instance";

  auto* globalThis = JSContextGetGlobalObject(ctx);
  std::vector<std::string> argumentNames(args.size());
  for (size_t i = 0; i < args.size(); i++) {
    argumentNames[i] = flatNamespace + "_arg" + std::to_string(i);
    JSObjectSetProperty(ctx, globalThis, JscStringRAII(argumentNames[i].c_str()), args[i],
                        kJSPropertyAttributeNone, exception);
  }
  replaceNewClassInstanceStatementInPlace(source, instanceName, argumentNames);

  auto sourceUrl = JscStringRAII(filePath.generic_string().c_str());
  JSEvaluateScript(ctx, JscStringRAII(source.c_str()), nullptr, sourceUrl, 0, exception);
  if (*exception != nullptr) {
    LOG(ERROR) << "[jsc] Failed to evaluate script: " << filePath.filename().string();
    return nullptr;
  }

  const auto* instance =
      JSObjectGetProperty(ctx, globalThis, JscStringRAII(instanceName.c_str()), exception);
  if (*exception == nullptr) {
    return JSValueToObject(ctx, instance, exception);
  }
  return nullptr;
}

JSValueRef JscCodeLoader::loadEsmBundledModuleToGlobalThis(JSContextRef ctx,
                                                           const std::string& baseFolderPath,
                                                           const std::string& moduleName,
                                                           JSValueRef* exception) {
  auto [source, filePath] = loadModuleSource(ctx, baseFolderPath, moduleName);
  if (source.empty()) {
    const std::string message = "Failed to open file for module: " + moduleName;
    *exception = JSValueMakeString(ctx, JscStringRAII(message.c_str()));
    return nullptr;
  }

  JSEvaluateScript(ctx, JscStringRAII(source.c_str()), nullptr,
                   JscStringRAII(filePath.generic_string().c_str()), 0, exception);
  return JSContextGetGlobalObject(ctx);
}

JSValueRef JscCodeLoader::getExportedClassHavingMethodNameInModule(const JSContextRef ctx,
                                                                   const JSValueRef moduleObj,
                                                                   const char* methodName) {
  if (!JSValueIsObject(ctx, moduleObj)) {
    return JSValueMakeNull(ctx);
  }

  const JSObjectRef moduleObjRef = JSValueToObject(ctx, moduleObj, nullptr);

  // Get all properties of the module object
  const JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx, moduleObjRef);
  const size_t count = JSPropertyNameArrayGetCount(propertyNames);

  for (size_t i = 0; i < count; i++) {
    const JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
    if (const JSValueRef property = JSObjectGetProperty(ctx, moduleObjRef, propertyName, nullptr);
        JSValueIsObject(ctx, property)) {
      const JSObjectRef propertyObj = JSValueToObject(ctx, property, nullptr);
      const JSValueRef method =
          JSObjectGetProperty(ctx, propertyObj, JSStringCreateWithUTF8CString(methodName), nullptr);

      if (JSValueIsObject(ctx, method) &&
          JSObjectIsFunction(ctx, JSValueToObject(ctx, method, nullptr))) {
        JSPropertyNameArrayRelease(propertyNames);
        return property;
      }
    }
  }

  JSPropertyNameArrayRelease(propertyNames);
  return JSValueMakeNull(ctx);
}

JSValueRef JscCodeLoader::getExportedClassByNameInModule(const JSContextRef ctx,
                                                         const JSValueRef moduleObj,
                                                         const char* className) {
  if (!JSValueIsObject(ctx, moduleObj)) {
    return JSValueMakeNull(ctx);
  }

  const JSObjectRef moduleObjRef = JSValueToObject(ctx, moduleObj, nullptr);
  const JSValueRef classObj =
      JSObjectGetProperty(ctx, moduleObjRef, JscStringRAII(className), nullptr);
  return JSValueIsObject(ctx, classObj) ? classObj : JSValueMakeNull(ctx);
}

JSValueRef JscCodeLoader::getMethodByNameInClass(const JSContextRef ctx,
                                                 const JSValueRef classObj,
                                                 const char* methodName) {
  if (!JSValueIsObject(ctx, classObj)) {
    return JSValueMakeNull(ctx);
  }

  const JSObjectRef classObjRef = JSValueToObject(ctx, classObj, nullptr);
  if (const JSValueRef method =
          JSObjectGetProperty(ctx, classObjRef, JscStringRAII(methodName), nullptr);
      JSValueIsObject(ctx, method) &&
      JSObjectIsFunction(ctx, JSValueToObject(ctx, method, nullptr))) {
    return method;
  }

  return JSValueMakeNull(ctx);
}
