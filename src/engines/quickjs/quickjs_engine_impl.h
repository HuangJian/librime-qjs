#pragma once

#include <glog/logging.h>
#include <quickjs.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "engines/js_exception.h"
#include "patch/quickjs/node_module_loader.h"

// NEVER USE TEMPLATE IN THIS HEADER FILE
class QuickJsEngineImpl {
public:
  QuickJsEngineImpl();
  ~QuickJsEngineImpl();

  QuickJsEngineImpl(const QuickJsEngineImpl&) = delete;
  QuickJsEngineImpl(QuickJsEngineImpl&&) = delete;
  QuickJsEngineImpl& operator=(const QuickJsEngineImpl&) = delete;
  QuickJsEngineImpl& operator=(QuickJsEngineImpl&&) = delete;

  [[nodiscard]] JSContext* getContext() const { return context_; }

  [[nodiscard]] int64_t getMemoryUsage() const;
  [[nodiscard]] size_t getArrayLength(const JSValue& array) const;
  void insertItemToArray(JSValue array, size_t index, const JSValue& value) const;
  [[nodiscard]] JSValue getArrayItem(const JSValue& array, size_t index) const;
  [[nodiscard]] JSValue getObjectProperty(const JSValue& obj, const char* propertyName) const;
  int setObjectProperty(JSValue obj, const char* propertyName, const JSValue& value) const;
  int setObjectFunction(JSValue obj,
                        const char* functionName,
                        JSCFunction* cppFunction,
                        int expectingArgc) const;
  [[nodiscard]] JSValue callFunction(const JSValue& func,
                                     const JSValue& thisArg,
                                     int argc,
                                     JSValue* argv) const;
  [[nodiscard]] JSValue newClassInstance(const JSValue& clazz, int argc, JSValue* argv) const;
  [[nodiscard]] JSValue getJsClassHavingMethod(const JSValue& module, const char* methodName) const;
  [[nodiscard]] JSValue getMethodOfClassOrInstance(JSValue jsClass,
                                                   JSValue instance,
                                                   const char* methodName) const;
  void logErrorStackTrace(const JSValue& exception, const char* file, int line) const;

  [[nodiscard]] JSValue createInstanceOfModule(const char* moduleName,
                                               std::vector<JSValue>& args,
                                               const std::string& mainFuncName) const;
  [[nodiscard]] JSValue loadJsFile(const char* fileName) const;
  [[nodiscard]] JSValue eval(const char* code, const char* filename = "<eval>") const;
  [[nodiscard]] JSValue getGlobalObject() const;
  [[nodiscard]] JSValue throwError(JsErrorType errorType, const std::string& message) const;

  // Type conversion utilities
  [[nodiscard]] JSValue toJsString(const char* str) const { return JS_NewString(context_, str); }
  [[nodiscard]] JSValue toJsString(const std::string& str) const {
    return JS_NewString(context_, str.c_str());
  }
  [[nodiscard]] std::string toStdString(const JSValue& value) const;
  [[nodiscard]] JSValue toJsNumber(const double value) const {
    return JS_NewFloat64(context_, value);
  }
  [[nodiscard]] JSValue toJsNumber(const int64_t value) const {
    return JS_NewInt64(context_, value);
  }
  [[nodiscard]] double toDouble(const JSValue& value) const;
  [[nodiscard]] size_t toInt(const JSValue& value) const;

  void registerType(const char* typeName,
                    JSClassID& classId,
                    JSClassDef& classDef,
                    JSCFunction* constructor,
                    int constructorArgc,
                    JSClassFinalizer* finalizer,
                    const JSCFunctionListEntry* properties,
                    int propertyCount,
                    const JSCFunctionListEntry* getters,
                    int getterCount,
                    const JSCFunctionListEntry* functions,
                    int functionCount);

  [[nodiscard]] JSValue wrap(const char* typeName, void* ptr, const char* pointerType) const;

  void setBaseFolderPath(const char* absolutePath) {
    this->baseFolderPath_ = absolutePath;
    setQjsBaseFolder(absolutePath);
  }
  static void exposeLogToJsConsole(JSContext* ctx);

private:
  static JSValue jsLog(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);
  static JSValue jsError(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv);

  JSRuntime* runtime_;
  JSContext* context_;
  std::unordered_map<std::string, JSClassID> registeredTypes_;
  std::string baseFolderPath_;  // absolute path to the base folder of the js files
};
