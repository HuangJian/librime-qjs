#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <glog/logging.h>
#include <string>
#include <unordered_map>

// NEVER USE TEMPLATE IN THIS HEADER FILE
class JscEngineImpl {
public:
  JscEngineImpl();
  ~JscEngineImpl();

  JscEngineImpl(const JscEngineImpl& other) = default;
  JscEngineImpl(JscEngineImpl&&) = delete;
  JscEngineImpl& operator=(const JscEngineImpl&) = delete;
  JscEngineImpl& operator=(JscEngineImpl&&) = delete;

  [[nodiscard]] JSGlobalContextRef getContext() const { return ctx_; }

  [[nodiscard]] std::string toStdString(const JSValueRef& value) const;

  void setBaseFolderPath(const char* absolutePath);
  JSObjectRef createInstanceOfModule(const char* moduleName,
                                     const std::vector<JSValueRef>& args) const;
  JSValueRef loadJsFile(const char* fileName) const;
  JSValueRef eval(const char* code, const char* filename = "<eval>") const;
  JSObjectRef getGlobalObject() const;

  [[nodiscard]] size_t getArrayLength(const JSValueRef& array) const;
  void insertItemToArray(JSValueRef array, size_t index, const JSValueRef& value) const;
  [[nodiscard]] JSValueRef getArrayItem(const JSValueRef& array, size_t index) const;

  JSValueRef getObjectProperty(const JSObjectRef& obj, const char* propertyName) const;
  int setObjectProperty(const JSObjectRef& obj,
                        const char* propertyName,
                        const JSValueRef& value) const;
  int setObjectFunction(JSObjectRef obj,
                        const char* functionName,
                        JSObjectCallAsFunctionCallback cppFunction,
                        int expectingArgc) const;

  JSValueRef callFunction(const JSObjectRef& func,
                          const JSObjectRef& thisArg,
                          int argc,
                          const JSValueRef* argv) const;
  JSObjectRef newClassInstance(const JSObjectRef& clazz, int argc, const JSValueRef* argv) const;

  JSValueRef getJsClassHavingMethod(const JSValueRef& module, const char* methodName) const;
  JSObjectRef getMethodOfClassOrInstance(JSObjectRef jsClass,
                                         JSObjectRef instance,
                                         const char* methodName) const;

  void logErrorStackTrace(const JSValueRef& exception,
                          const char* file = __FILE_NAME__,
                          int line = __LINE__) const;

  void registerType(const char* typeName,
                    JSClassRef& jsClass,
                    JSObjectCallAsConstructorCallback constructor,
                    void (*finalizer)(JSObjectRef),
                    JSStaticFunction* functions,
                    int numFunctions,
                    const JSStaticValue* properties,
                    int numProperties,
                    const JSStaticValue* getters,
                    int numGetters);

  [[nodiscard]] bool isTypeRegistered(const std::string& typeName) const;

  [[nodiscard]] const JSClassRef& getRegisteredClass(const std::string& typeName) const;

  static void exposeLogToJsConsole(JSContextRef ctx);

private:
  static JSValueRef jsLog(JSContextRef ctx,
                          JSObjectRef function,
                          JSObjectRef thisObject,
                          size_t argumentCount,
                          const JSValueRef arguments[],
                          JSValueRef* exception);

  static JSValueRef jsError(JSContextRef ctx,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef* exception);
  JSGlobalContextRef ctx_{nullptr};
  std::vector<std::string> arrBaseFolderPath_;
  std::unordered_map<std::string, JSClassRef> clazzes_;
};
