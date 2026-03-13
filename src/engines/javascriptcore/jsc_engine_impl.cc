#include "engines/javascriptcore/jsc_engine_impl.h"
#include "engines/javascriptcore/jsc_code_loader.h"
#include "engines/javascriptcore/jsc_string_raii.hpp"

JscEngineImpl::JscEngineImpl() : ctx_(JSGlobalContextCreate(nullptr)) {
  exposeLogToJsConsole(ctx_);
}

JscEngineImpl::~JscEngineImpl() {
  for (auto& [key, val] : clazzes_) {
    auto& clazzDef = val;
    JSClassRelease(clazzDef);
  }
  JSGlobalContextRelease(ctx_);
}

void JscEngineImpl::setBaseFolderPath(const char* absolutePath) {
  // Check for duplicates
  for (const auto& path : arrBaseFolderPath_) {
    if (path == absolutePath) {
      DLOG(INFO) << "[jsc] Base folder already registered: " << absolutePath;
      return;
    }
  }
  arrBaseFolderPath_.emplace_back(absolutePath);
}

JSObjectRef JscEngineImpl::createInstanceOfModule(const char* moduleName,
                                                  const std::vector<JSValueRef>& args) const {
  JSValueRef exception = nullptr;
  for (auto& path : arrBaseFolderPath_) {
    const JSObjectRef instance =
        JscCodeLoader::createInstanceOfIifeBundledModule(ctx_, path, moduleName, args, &exception);
    if (exception == nullptr) {
      return instance;
    }
  }
  if (exception != nullptr) {
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
  }

  return nullptr;
}

JSValueRef JscEngineImpl::loadJsFile(const char* fileName) const {
  JSValueRef exception = nullptr;

  for (auto& path : arrBaseFolderPath_) {
    const auto* globalThis =
        JscCodeLoader::loadEsmBundledModuleToGlobalThis(ctx_, path, fileName, &exception);
    if (exception == nullptr) {
      return globalThis;
    }
  }
  logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
  return nullptr;
}

JSValueRef JscEngineImpl::eval(const char* code, const char* filename) const {
  const auto jsCode = JscStringRAII(code);
  const auto filenameStr = JscStringRAII(filename);
  JSValueRef exception = nullptr;
  const JSValueRef result = JSEvaluateScript(ctx_, jsCode, nullptr, filenameStr, 0, &exception);
  logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
  return result;
}

JSObjectRef JscEngineImpl::getGlobalObject() const {
  return JSContextGetGlobalObject(ctx_);
}

size_t JscEngineImpl::getArrayLength(const JSValueRef& array) const {
  JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
  JSValueRef lengthValue = JSObjectGetProperty(ctx_, arrayObj, JscStringRAII("length"), nullptr);
  return static_cast<size_t>(JSValueToNumber(ctx_, lengthValue, nullptr));
}

void JscEngineImpl::insertItemToArray(const JSValueRef array,
                                      const size_t index,
                                      const JSValueRef& value) const {
  JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
  JSObjectSetPropertyAtIndex(ctx_, arrayObj, index, value, nullptr);
}

JSValueRef JscEngineImpl::getArrayItem(const JSValueRef& array, const size_t index) const {
  const JSObjectRef arrayObj = JSValueToObject(ctx_, array, nullptr);
  return JSObjectGetPropertyAtIndex(ctx_, arrayObj, index, nullptr);
}

JSValueRef JscEngineImpl::getObjectProperty(const JSObjectRef& obj,
                                            const char* propertyName) const {
  return JSObjectGetProperty(ctx_, obj, JscStringRAII(propertyName), nullptr);
}

int JscEngineImpl::setObjectProperty(const JSObjectRef& obj,
                                     const char* propertyName,
                                     const JSValueRef& value) const {
  JSObjectSetProperty(ctx_, obj, JscStringRAII(propertyName), value, kJSPropertyAttributeNone,
                      nullptr);
  return 0;
}

int JscEngineImpl::setObjectFunction(const JSObjectRef obj,
                                     const char* functionName,
                                     JSObjectCallAsFunctionCallback cppFunction,
                                     [[maybe_unused]] int expectingArgc) const {
  const auto funcNameStr = JscStringRAII(functionName);
  const JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx_, funcNameStr, cppFunction);
  JSObjectSetProperty(ctx_, obj, funcNameStr, func, kJSPropertyAttributeNone, nullptr);
  return 0;
}

JSValueRef JscEngineImpl::callFunction(const JSObjectRef& func,
                                       const JSObjectRef& thisArg,
                                       const int argc,
                                       const JSValueRef* argv) const {
  JSValueRef exception = nullptr;
  auto* thisVal = JSValueIsUndefined(ctx_, thisArg) ? getGlobalObject() : thisArg;
  const JSValueRef result = JSObjectCallAsFunction(ctx_, func, thisVal, argc, argv, &exception);
  if (exception != nullptr) {
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return nullptr;
  }
  return result;
}

JSObjectRef JscEngineImpl::newClassInstance(const JSObjectRef& clazz,
                                            const int argc,
                                            const JSValueRef* argv) const {
  JSValueRef exception = nullptr;
  const JSObjectRef result = JSObjectCallAsConstructor(ctx_, clazz, argc, argv, &exception);
  if (exception != nullptr) {
    logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
    return nullptr;
  }
  return result;
}

JSValueRef JscEngineImpl::getJsClassHavingMethod(const JSValueRef& module,
                                                 const char* methodName) const {
  const JSObjectRef moduleObj = JSValueToObject(ctx_, module, nullptr);
  const JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(ctx_, moduleObj);
  const size_t count = JSPropertyNameArrayGetCount(propertyNames);

  for (int i = count - 1; i >= 0; i--) {
    const JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(propertyNames, i);
    if (const JSValueRef value = JSObjectGetProperty(ctx_, moduleObj, propertyName, nullptr);
        JSValueIsObject(ctx_, value)) {
      const JSObjectRef obj = JSValueToObject(ctx_, value, nullptr);

      if (const JSValueRef prototype =
              JSObjectGetProperty(ctx_, obj, JscStringRAII("prototype"), nullptr);
          !JSValueIsUndefined(ctx_, prototype) && JSValueIsObject(ctx_, prototype)) {
        const JSObjectRef prototypeObj = JSValueToObject(ctx_, prototype, nullptr);
        const JSValueRef method =
            JSObjectGetProperty(ctx_, prototypeObj, JscStringRAII(methodName), nullptr);

        if (!JSValueIsUndefined(ctx_, method) && JSValueIsObject(ctx_, method)) {
          JSPropertyNameArrayRelease(propertyNames);
          return value;
        }
      }
    }
  }

  JSPropertyNameArrayRelease(propertyNames);
  return JSValueMakeUndefined(ctx_);
}

JSObjectRef JscEngineImpl::getMethodOfClassOrInstance([[maybe_unused]] JSObjectRef jsClass,
                                                      const JSObjectRef instance,
                                                      const char* methodName) const {
  JSValueRef exception = nullptr;
  const JSValueRef method =
      JSObjectGetProperty(ctx_, instance, JscStringRAII(methodName), &exception);
  logErrorStackTrace(exception, __FILE_NAME__, __LINE__);
  return JSValueToObject(ctx_, method, nullptr);
}

void JscEngineImpl::logErrorStackTrace(const JSValueRef& exception,
                                       const char* file,
                                       const int line) const {
  if (exception == nullptr) {
    return;
  }

  auto* objException = JSValueToObject(ctx_, exception, nullptr);
  const auto strStack = toStdString(getObjectProperty(objException, "stack"));
  google::LogMessage(file, line, google::GLOG_ERROR).stream()
      << "[qjs] JSC exception: " << toStdString(exception) << "\n"
      << (strStack.empty() ? "No stack trace available." : strStack);
}

std::string JscEngineImpl::toStdString(const JSValueRef& value) const {
  if ((ctx_ == nullptr) || (value == nullptr)) {
    return {};
  }

  JSStringRef jsString = JSValueToStringCopy(ctx_, value, nullptr);
  if (jsString == nullptr) {
    return {};
  }

  const size_t maxBufferSize = JSStringGetMaximumUTF8CStringSize(jsString);
  std::string result;
  result.resize(maxBufferSize);

  const size_t actualSize = JSStringGetUTF8CString(jsString, result.data(), maxBufferSize);
  result.resize(actualSize > 0 ? actualSize - 1 : 0);  // Remove null terminator if present

  JSStringRelease(jsString);
  return result;
}

void JscEngineImpl::registerType(const char* typeName,
                                 JSClassRef& jsClass,
                                 const JSObjectCallAsConstructorCallback constructor,
                                 void (*finalizer)(JSObjectRef),
                                 JSStaticFunction* functions,
                                 int numFunctions,
                                 const JSStaticValue* properties,
                                 const int numProperties,
                                 const JSStaticValue* getters,
                                 const int numGetters) {
  if (clazzes_.find(typeName) != clazzes_.end()) {
    DLOG(INFO) << "[jsc] type: " << typeName << " has already been registered.";
    return;
  }
  DLOG(INFO) << "[jsc] registering type: " << typeName;

  std::vector<JSStaticValue> staticValues;
  staticValues.reserve(numProperties + numGetters + 1);
  for (int i = 0; i < numProperties; i++) {
    staticValues.push_back(properties[i]);
  }
  for (int i = 0; i < numGetters; i++) {
    staticValues.push_back(getters[i]);
  }
  staticValues.push_back({nullptr, nullptr, nullptr, 0});

  const JSClassDefinition classDef = {.version = 0,
                                      .attributes = kJSClassAttributeNone,
                                      .className = typeName,
                                      .parentClass = nullptr,
                                      .staticValues = staticValues.data(),
                                      .staticFunctions = functions,
                                      .initialize = nullptr,
                                      .finalize = finalizer,
                                      .hasProperty = nullptr,
                                      .getProperty = nullptr,
                                      .setProperty = nullptr,
                                      .deleteProperty = nullptr,
                                      .getPropertyNames = nullptr,
                                      .callAsFunction = nullptr,
                                      .callAsConstructor = constructor,
                                      .hasInstance = nullptr,
                                      .convertToType = nullptr};

  DLOG(INFO) << "[jsc] registering type: " << typeName << " with " << (staticValues.size() - 1)
             << " properties and " << numFunctions << " functions";

  jsClass = JSClassCreate(&classDef);
  clazzes_[typeName] = jsClass;

  // Add the constructor to the global object
  const JSObjectRef globalObj = JSContextGetGlobalObject(ctx_);
  const JSObjectRef constructorObj = JSObjectMake(ctx_, jsClass, nullptr);
  JSObjectSetProperty(ctx_, globalObj, JscStringRAII(typeName), constructorObj,
                      kJSPropertyAttributeNone, nullptr);
}

bool JscEngineImpl::isTypeRegistered(const std::string& typeName) const {
  if (clazzes_.find(typeName) == clazzes_.end()) {
    LOG(ERROR) << "type: " << typeName << " has not been registered.";
    return false;
  }
  return true;
}

const JSClassRef& JscEngineImpl::getRegisteredClass(const std::string& typeName) const {
  if (!isTypeRegistered(typeName)) {
    throw std::runtime_error("type: " + typeName + " has not been registered.");
  }
  return clazzes_.at(typeName);
}

void JscEngineImpl::exposeLogToJsConsole(JSContextRef ctx) {
  const JSObjectRef globalObject = JSContextGetGlobalObject(ctx);

  // Create console object
  const JSObjectRef consoleObj = JSObjectMake(ctx, nullptr, nullptr);

  // Create log function
  const auto logStr = JscStringRAII("log");
  const JSObjectRef logFunc = JSObjectMakeFunctionWithCallback(ctx, logStr, jsLog);

  // Create error function
  const auto errorStr = JscStringRAII("error");
  const JSObjectRef errorFunc = JSObjectMakeFunctionWithCallback(ctx, errorStr, jsError);

  // Add functions to console object
  JSObjectSetProperty(ctx, consoleObj, logStr, logFunc, kJSPropertyAttributeNone, nullptr);
  JSObjectSetProperty(ctx, consoleObj, errorStr, errorFunc, kJSPropertyAttributeNone, nullptr);

  // Add console object to global scope
  JSObjectSetProperty(ctx, globalObject, JscStringRAII("console"), consoleObj,
                      kJSPropertyAttributeNone, nullptr);
}

static std::string processJsArguments(const JSContextRef ctx,
                                      const size_t argumentCount,
                                      const JSValueRef arguments[],
                                      JSValueRef* exception) {
  std::stringstream ss;
  for (size_t i = 0; i < argumentCount; i++) {
    auto strRef = JscStringRAII(JSValueToStringCopy(ctx, arguments[i], exception));
    const size_t bufferSize = JSStringGetMaximumUTF8CStringSize(strRef);
    auto* buffer = new char[bufferSize];
    JSStringGetUTF8CString(strRef, buffer, bufferSize);
    ss << buffer;
    if (i < argumentCount - 1) {
      ss << " ";
    }
    delete[] buffer;
  }
  return ss.str();
}

JSValueRef JscEngineImpl::jsLog(const JSContextRef ctx,
                                [[maybe_unused]] JSObjectRef function,
                                [[maybe_unused]] JSObjectRef thisObject,
                                const size_t argumentCount,
                                const JSValueRef arguments[],
                                JSValueRef* exception) {
  std::string message = processJsArguments(ctx, argumentCount, arguments, exception);
  google::LogMessage("$jsc$", 0, google::GLOG_INFO).stream() << message;
  return JSValueMakeUndefined(ctx);
}

JSValueRef JscEngineImpl::jsError(const JSContextRef ctx,
                                  [[maybe_unused]] JSObjectRef function,
                                  [[maybe_unused]] JSObjectRef thisObject,
                                  const size_t argumentCount,
                                  const JSValueRef arguments[],
                                  JSValueRef* exception) {
  const std::string message = processJsArguments(ctx, argumentCount, arguments, exception);
  google::LogMessage("$jsc$", 0, google::GLOG_ERROR).stream() << message;
  return JSValueMakeUndefined(ctx);
}
