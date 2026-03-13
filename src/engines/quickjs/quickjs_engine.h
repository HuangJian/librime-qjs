#pragma once

#include <quickjs.h>
#include <memory>
#include <mutex>

#include "engines/js_exception.h"
#include "engines/js_traits.h"
#include "engines/quickjs/quickjs_engine_impl.h"
#include "types/js_wrapper.h"

template <typename T_JS_VALUE>
class JsEngine;

template <>
class JsEngine<JSValue> {
  inline static std::mutex instanceMutex;
  inline static bool isInitialized = false;

  std::unique_ptr<QuickJsEngineImpl> impl_{std::make_unique<QuickJsEngineImpl>()};

  JsEngine() { isInitialized = true; };

public:
  using T_JS_OBJECT = JSValue;
  inline static auto engineName = "QuickJS-NG";

  ~JsEngine() = default;

  JsEngine(const JsEngine& other) = delete;
  JsEngine(JsEngine&&) = delete;
  JsEngine& operator=(const JsEngine& other) = delete;
  JsEngine& operator=(JsEngine&&) = delete;

  static JsEngine& instance() {
    std::lock_guard lock(instanceMutex);
    static JsEngine instance;
    return instance;
  }

  static void setup() {
    // the QuickJsEngineImpl object would be created in `instance()` or `shutdown()`
  }
  static void shutdown() {
    if (!isInitialized) {
      return;
    }

    auto& sharedInstance = instance();

    std::lock_guard lock(instanceMutex);
    sharedInstance.impl_ = std::make_unique<QuickJsEngineImpl>();
  }

  [[nodiscard]] int64_t getMemoryUsage() const { return impl_->getMemoryUsage(); }

  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  [[nodiscard]] JSValue null() const { return JS_NULL; }
  [[nodiscard]] JSValue undefined() const { return JS_UNDEFINED; }

  [[nodiscard]] JSValue jsTrue() const { return JS_TRUE; }
  [[nodiscard]] JSValue jsFalse() const { return JS_FALSE; }

  [[nodiscard]] bool isArray(const JSValue& value) const { return JS_IsArray(value); }
  [[nodiscard]] bool isObject(const JSValue& value) const { return JS_IsObject(value); }
  [[nodiscard]] bool isBool(const JSValue& value) const { return JS_IsBool(value); }
  [[nodiscard]] bool isNull(const JSValue& value) const { return JS_IsNull(value); }
  [[nodiscard]] bool isUndefined(const JSValue& value) const { return JS_IsUndefined(value); }
  [[nodiscard]] bool isException(const JSValue& value) const { return JS_IsException(value); }

  [[nodiscard]] JSValue toObject(const JSValue& value) const { return value; }

  void setBaseFolderPath(const char* absolutePath) const { impl_->setBaseFolderPath(absolutePath); }
  // NOLINTEND(readability-convert-member-functions-to-static)

  [[nodiscard]] JSValue newArray() const { return JS_NewArray(impl_->getContext()); }

  [[nodiscard]] size_t getArrayLength(const JSValue& array) const {
    return impl_->getArrayLength(array);
  }

  void insertItemToArray(const JSValue array, const size_t index, const JSValue& value) const {
    impl_->insertItemToArray(array, index, value);
  }

  [[nodiscard]] JSValue getArrayItem(const JSValue& array, const size_t index) const {
    return impl_->getArrayItem(array, index);
  }

  [[nodiscard]] JSValue newObject() const { return JS_NewObject(impl_->getContext()); }

  [[nodiscard]] JSValue getObjectProperty(const JSValue& obj, const char* propertyName) const {
    return impl_->getObjectProperty(obj, propertyName);
  }

  int setObjectProperty(JSValue obj, const char* propertyName, const JSValue& value) const {
    return impl_->setObjectProperty(obj, propertyName, value);
  }

  using ExposeFunction = JSCFunction*;
  int setObjectFunction(const JSValue obj,
                        const char* functionName,
                        const ExposeFunction cppFunction,
                        const int expectingArgc) const {
    return impl_->setObjectFunction(obj, functionName, cppFunction, expectingArgc);
  }

  [[nodiscard]] std::string toStdString(const JSValue& value) const {
    return impl_->toStdString(value);
  }

  [[nodiscard]] bool toBool(const JSValue& value) const {
    return JS_ToBool(impl_->getContext(), value) != 0;
  }

  [[nodiscard]] size_t toInt(const JSValue& value) const { return impl_->toInt(value); }

  [[nodiscard]] double toDouble(const JSValue& value) const { return impl_->toDouble(value); }

  [[nodiscard]] JSValue callFunction(const JSValue& func,
                                     const JSValue& thisArg,
                                     const int argc,
                                     JSValue* argv) const {
    return impl_->callFunction(func, thisArg, argc, argv);
  }

  [[nodiscard]] JSValue newClassInstance(const JSValue& clazz,
                                         const int argc,
                                         JSValue* argv) const {
    return impl_->newClassInstance(clazz, argc, argv);
  }

  [[nodiscard]] JSValue getJsClassHavingMethod(const JSValue& module,
                                               const char* methodName) const {
    return impl_->getJsClassHavingMethod(module, methodName);
  }

  [[nodiscard]] JSValue getMethodOfClassOrInstance(const JSValue jsClass,
                                                   const JSValue instance,
                                                   const char* methodName) const {
    return impl_->getMethodOfClassOrInstance(jsClass, instance, methodName);
  }

  [[nodiscard]] bool isFunction(const JSValue& value) const {
    return JS_IsFunction(impl_->getContext(), value);
  }
  [[nodiscard]] JSValue getLatestException() const { return JS_GetException(impl_->getContext()); }

  void logErrorStackTrace(const JSValue& exception, const char* file, const int line) const {
    impl_->logErrorStackTrace(exception, file, line);
  }

  [[nodiscard]] JSValue duplicateValue(const JSValue& value) const {
    return JS_DupValue(impl_->getContext(), value);
  }

  template <typename... Args>
  void freeValue(const Args&... args) const {
    (JS_FreeValue(impl_->getContext(), args), ...);
  }

  template <typename... Args>
  void protectFromGC(const Args&... args) const {
    // quickjs uses reference counting, so we don't need to protect from GC
  }

  template <typename... Args>
  void unprotectFromGC(const Args&... args) const {
    // quickjs uses reference counting, so we don't need to protect from GC
  }

  template <typename T_RIME_TYPE>
  void registerType() {
    using WRAPPER = JsWrapper<T_RIME_TYPE>;
    impl_->registerType(WRAPPER::typeName, WRAPPER::jsClassId, WRAPPER::JS_CLASS_DEF,
                        WRAPPER::constructorQjs, WRAPPER::CONSTRUCTOR_ARGC, WRAPPER::finalizerQjs,
                        WRAPPER::PROPERTIES_QJS, WRAPPER::PROPERTIES_SIZE, WRAPPER::GETTERS_QJS,
                        WRAPPER::GETTERS_SIZE, WRAPPER::FUNCTIONS_QJS, WRAPPER::FUNCTIONS_SIZE);
  }

  template <typename T>
  [[nodiscard]] typename JsWrapper<T>::T_UNWRAP_TYPE unwrap(const JSValue& value) const {
    if constexpr (is_shared_ptr_v<typename JsWrapper<T>::T_UNWRAP_TYPE>) {
      if (auto* ptr = JS_GetOpaque(value, JsWrapper<T>::jsClassId)) {
        auto sharedPtr = static_cast<std::shared_ptr<T>*>(ptr);
        return *sharedPtr;
      }
    } else {
      if (auto* ptr = JS_GetOpaque(value, JsWrapper<T>::jsClassId)) {
        return static_cast<T*>(ptr);
      }
    }
    return nullptr;
  }

  template <typename T>
  [[nodiscard]] std::enable_if_t<!is_shared_ptr_v<T>, JSValue> wrap(T* ptrValue) const {
    using DereferencedType = std::decay_t<decltype(*ptrValue)>;
    return impl_->wrap(JsWrapper<DereferencedType>::typeName, ptrValue, "raw");
  }

  template <typename T>
  [[nodiscard]] std::enable_if_t<is_shared_ptr_v<T>, JSValue> wrap(T ptrValue) const {
    if (ptrValue == nullptr) {
      return JS_NULL;
    }
    using Inner = shared_ptr_inner_t<decltype(ptrValue)>;
    auto ptr = std::make_unique<std::shared_ptr<Inner>>(ptrValue);
    return impl_->wrap(JsWrapper<Inner>::typeName, ptr.release(), "shared");
  }

  [[nodiscard]] JSValue wrap(const char* str) const { return impl_->toJsString(str); }
  [[nodiscard]] JSValue wrap(const std::string& str) const { return impl_->toJsString(str); }
  [[nodiscard]] JSValue wrap(const bool value) const {
    return JS_NewBool(impl_->getContext(), value);
  }
  [[nodiscard]] JSValue wrap(const size_t value) const {
    return impl_->toJsNumber(static_cast<int64_t>(value));
  }
  [[nodiscard]] JSValue wrap(const int value) const {
    return impl_->toJsNumber(static_cast<int64_t>(value));
  }
  [[nodiscard]] JSValue wrap(const double value) const { return impl_->toJsNumber(value); }

  [[nodiscard]] JSValue createInstanceOfModule(const char* moduleName,
                                               std::vector<JSValue>& args,
                                               const std::string& mainFuncName) const {
    return impl_->createInstanceOfModule(moduleName, args, mainFuncName);
  }
  [[nodiscard]] JSValue loadJsFile(const char* fileName) const {
    return impl_->loadJsFile(fileName);
  }

  [[nodiscard]] JSValue eval(const char* code, const char* filename = "<eval>") const {
    return JS_Eval(impl_->getContext(), code, strlen(code), filename, JS_EVAL_TYPE_GLOBAL);
  }

  [[nodiscard]] JSValue getGlobalObject() const { return JS_GetGlobalObject(impl_->getContext()); }

  [[nodiscard]] JSValue throwError(const JsErrorType errorType, const std::string& message) const {
    return impl_->throwError(errorType, message);
  }
};
