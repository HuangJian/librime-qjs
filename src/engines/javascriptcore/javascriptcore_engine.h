#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <memory>
#include <mutex>

#include "engines/javascriptcore/jsc_engine_impl.h"
#include "engines/javascriptcore/jsc_string_raii.hpp"
#include "engines/js_exception.h"
#include "engines/js_traits.h"
#include "types/js_wrapper.h"

#include "engines/quickjs/quickjs_engine.h"

template <>
class JsEngine<JSValueRef> {
  inline static bool isInitialized = false;
  inline static std::mutex instanceMutex;
  std::unique_ptr<JscEngineImpl> impl_{std::make_unique<JscEngineImpl>()};

  JsEngine<JSValueRef>() = default;

public:
  using T_JS_OBJECT = JSObjectRef;
  inline static const char* engineName = "JavaScriptCore";

  static JsEngine<JSValueRef>& instance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    static JsEngine<JSValueRef> instance;
    return instance;
  }

  static void setup() {
    // the options could be found with the command: `jsc -options`
    setenv("JSC_dumpOptions", "1", 1);  // Logs Overridden JSC runtime options at startup
    setenv("JSC_useConcurrentJIT", "true",
           1);  // allows the DFG / FTL compilation in threads other than the executing JS thread

    // options for DFG optimization
    setenv("JSC_useDFGJIT", "true", 1);
    setenv("JSC_thresholdForJITSoon", "1", 1);
    setenv("JSC_thresholdForJITAfterWarmUp", "1", 1);
    setenv("JSC_thresholdForOptimizeAfterWarmUp", "1", 1);
    setenv("JSC_thresholdForOptimizeAfterLongWarmUp", "1", 1);
    setenv("JSC_thresholdForOptimizeSoon", "1", 1);
    // setenv("JSC_reportDFGCompileTimes", "true", 1);

    // options for FTL optimization
    setenv("JSC_useFTLJIT", "true", 1);
    setenv("JSC_thresholdForFTLOptimizeAfterWarmUp", "1", 1);
    setenv("JSC_thresholdForFTLOptimizeSoon", "1", 1);
    setenv("JSC_reportFTLCompileTimes", "true", 1);
  }

  static void shutdown() {
    if (!isInitialized) {
      return;
    }
    auto& sharedInstance = instance();

    std::lock_guard<std::mutex> lock(instanceMutex);
    sharedInstance.impl_ = std::make_unique<JscEngineImpl>();
  }

  ~JsEngine<JSValueRef>() = default;

  JsEngine(const JsEngine&) = delete;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine& other) = delete;
  JsEngine& operator=(JsEngine&&) = delete;

  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  int64_t getMemoryUsage() {
    // TODO: find an API to get the memory usage of JavaScriptCore
    return -1;
  }

  [[nodiscard]] bool isException(const JSValueRef& value) const {
    // nullptr is returned in `callFunction` and `newClassInstance` if an exception is thrown
    return value == nullptr;
  }

  [[nodiscard]] JSValueRef duplicateValue(const JSValueRef& value) const {
    // JavaScriptCore handles memory management automatically
    return value;
  }

  template <typename... Args>
  void freeValue(const Args&... args) const {
    // JavaScriptCore handles memory management automatically
  }

  template <typename... Args>
  void protectFromGC(const Args&... args) const {
    (((args != nullptr && !isUndefined(args)) ? JSValueProtect(impl_->getContext(), args) : void()),
     ...);
  }

  template <typename... Args>
  void unprotectFromGC(const Args&... args) const {
    (((args != nullptr && !isUndefined(args)) ? JSValueUnprotect(impl_->getContext(), args)
                                              : void()),
     ...);
  }

  // NOLINTEND(readability-convert-member-functions-to-static)
  [[nodiscard]] JSValueRef null() const { return JSValueMakeNull(impl_->getContext()); }
  [[nodiscard]] JSValueRef undefined() const { return JSValueMakeUndefined(impl_->getContext()); }

  [[nodiscard]] JSValueRef jsTrue() const { return JSValueMakeBoolean(impl_->getContext(), true); }
  [[nodiscard]] JSValueRef jsFalse() const {
    return JSValueMakeBoolean(impl_->getContext(), false);
  }

  [[nodiscard]] JSValueRef newArray() const {
    return JSObjectMakeArray(impl_->getContext(), 0, nullptr, nullptr);
  }

  [[nodiscard]] size_t getArrayLength(const JSValueRef& array) const {
    return impl_->getArrayLength(array);
  }

  void insertItemToArray(JSValueRef array, size_t index, const JSValueRef& value) const {
    impl_->insertItemToArray(array, index, value);
  }

  [[nodiscard]] JSValueRef getArrayItem(const JSValueRef& array, size_t index) const {
    return impl_->getArrayItem(array, index);
  }

  [[nodiscard]] JSObjectRef newObject() const {
    return JSObjectMake(impl_->getContext(), nullptr, nullptr);
  }

  [[nodiscard]] JSValueRef getObjectProperty(const JSObjectRef& obj,
                                             const char* propertyName) const {
    return impl_->getObjectProperty(obj, propertyName);
  }

  int setObjectProperty(const JSObjectRef& obj, const char* propertyName, const JSValueRef& value) {
    return impl_->setObjectProperty(obj, propertyName, value);
  }

  int setObjectFunction(JSObjectRef obj,
                        const char* functionName,
                        JSObjectCallAsFunctionCallback cppFunction,
                        int expectingArgc) {
    return impl_->setObjectFunction(obj, functionName, cppFunction, expectingArgc);
  }

  [[nodiscard]] JSObjectRef toObject(const JSValueRef& value) const {
    if (value == nullptr) {
      return nullptr;
    }
    return JSValueToObject(impl_->getContext(), value, nullptr);
  }

  [[nodiscard]] std::string toStdString(const JSValueRef& value) const {
    return impl_->toStdString(value);
  }

  [[nodiscard]] bool toBool(const JSValueRef& value) const {
    return JSValueToBoolean(impl_->getContext(), value);
  }

  [[nodiscard]] size_t toInt(const JSValueRef& value) const {
    return static_cast<size_t>(JSValueToNumber(impl_->getContext(), value, nullptr));
  }

  [[nodiscard]] double toDouble(const JSValueRef& value) const {
    return JSValueToNumber(impl_->getContext(), value, nullptr);
  }

  JSValueRef callFunction(const JSObjectRef& func,
                          const JSObjectRef& thisArg,
                          int argc,
                          JSValueRef* argv) const {
    return impl_->callFunction(func, thisArg, argc, argv);
  }

  JSObjectRef newClassInstance(const JSObjectRef& clazz, int argc, JSValueRef* argv) {
    return impl_->newClassInstance(clazz, argc, argv);
  }

  JSValueRef getJsClassHavingMethod(const JSValueRef& module, const char* methodName) const {
    return impl_->getJsClassHavingMethod(module, methodName);
  }

  JSObjectRef getMethodOfClassOrInstance(JSObjectRef jsClass,
                                         JSObjectRef instance,
                                         const char* methodName) {
    return impl_->getMethodOfClassOrInstance(jsClass, instance, methodName);
  }

  [[nodiscard]] JSValueRef throwError(JsErrorType errorType, const std::string& message) const {
    throw JsException(errorType, message);
  }

  [[nodiscard]] bool isFunction(const JSValueRef& value) const {
    return isObject(value) && JSObjectIsFunction(impl_->getContext(), toObject(value));
  }

  [[nodiscard]] bool isArray(const JSValueRef& value) const {
    return JSValueIsArray(impl_->getContext(), value);
  }

  [[nodiscard]] bool isObject(const JSValueRef& value) const {
    return value != nullptr && JSValueIsObject(impl_->getContext(), value);
  }

  [[nodiscard]] bool isBool(const JSValueRef& value) const {
    return JSValueIsBoolean(impl_->getContext(), value);
  }

  [[nodiscard]] bool isNull(const JSValueRef& value) const {
    return JSValueIsNull(impl_->getContext(), value);
  }

  [[nodiscard]] bool isUndefined(const JSValueRef& value) const {
    return JSValueIsUndefined(impl_->getContext(), value);
  }

  template <typename T_RIME_TYPE>
  void registerType() {
    using WRAPPER = JsWrapper<T_RIME_TYPE>;
    impl_->registerType(WRAPPER::typeName, WRAPPER::classDefJsc, WRAPPER::constructorJsc,
                        WRAPPER::finalizerJsc, WRAPPER::functionsJsc, WRAPPER::FUNCTIONS_SIZE,
                        WRAPPER::propertiesJsc, WRAPPER::PROPERTIES_SIZE, WRAPPER::gettersJsc,
                        WRAPPER::GETTERS_SIZE);
  }

  template <typename T>
  [[nodiscard]] bool isTypeRegistered() const {
    return impl_->isTypeRegistered(JsWrapper<T>::typeName);
  }

  template <typename T>
  [[nodiscard]] typename JsWrapper<T>::T_UNWRAP_TYPE unwrap(const JSValueRef& value) const {
    if (!value || !isTypeRegistered<T>() || !isObject(value)) {
      return nullptr;
    }

    if constexpr (is_shared_ptr_v<typename JsWrapper<T>::T_UNWRAP_TYPE>) {
      if (void* ptr = JSObjectGetPrivate(toObject(value))) {
        if (auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr)) {
          return *sharedPtr;
        }
      }
    } else {
      if (auto* ptr = JSObjectGetPrivate(toObject(value))) {
        return static_cast<T*>(ptr);
      }
    }
    return nullptr;
  }

  template <typename T>
  std::enable_if_t<!is_shared_ptr_v<T>, JSObjectRef> wrap(T ptrValue) const {
    using DereferencedType = std::decay_t<decltype(*ptrValue)>;
    if (!ptrValue || !isTypeRegistered<DereferencedType>()) {
      return nullptr;
    }

    auto typeName = JsWrapper<DereferencedType>::typeName;
    const JSClassRef& jsClass = impl_->getRegisteredClass(typeName);
    return JSObjectMake(impl_->getContext(), jsClass, ptrValue);
  }

  template <typename T>
  std::enable_if_t<is_shared_ptr_v<T>, JSObjectRef> wrap(T ptrValue) const {
    using Inner = shared_ptr_inner_t<decltype(ptrValue)>;
    if (!ptrValue || !isTypeRegistered<Inner>()) {
      return nullptr;
    }

    const JSClassRef& jsClass = impl_->getRegisteredClass(JsWrapper<Inner>::typeName);
    auto ptr = std::make_unique<std::shared_ptr<Inner>>(ptrValue);
    return JSObjectMake(impl_->getContext(), jsClass, ptr.release());
  }

  [[nodiscard]] JSValueRef wrap(const char* str) const {
    return JSValueMakeString(impl_->getContext(), JscStringRAII(str));
  }
  [[nodiscard]] JSValueRef wrap(const std::string& str) const { return wrap(str.c_str()); }
  [[nodiscard]] JSValueRef wrap(bool value) const {
    return JSValueMakeBoolean(impl_->getContext(), value);
  }
  [[nodiscard]] JSValueRef wrap(size_t value) const {
    return JSValueMakeNumber(impl_->getContext(), static_cast<double>(value));
  }
  [[nodiscard]] JSValueRef wrap(int value) const {
    return JSValueMakeNumber(impl_->getContext(), static_cast<double>(value));
  }
  [[nodiscard]] JSValueRef wrap(double value) const {
    return JSValueMakeNumber(impl_->getContext(), value);
  }

  void setBaseFolderPath(const char* absolutePath) { impl_->setBaseFolderPath(absolutePath); }

  JSObjectRef createInstanceOfModule(const char* moduleName,
                                     const std::vector<JSValueRef>& args,
                                     const std::string& mainFuncName) {
    return impl_->createInstanceOfModule(moduleName, args);
  }

  JSValueRef loadJsFile(const char* fileName) { return impl_->loadJsFile(fileName); }

  JSValueRef eval(const char* code, const char* filename = "<eval>") {
    return impl_->eval(code, filename);
  }

  JSValueRef getGlobalObject() { return impl_->getGlobalObject(); }
};
