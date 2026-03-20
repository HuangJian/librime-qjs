#pragma once

#include <quickjs.h>
#include <memory>
#include <mutex>
#include <vector>

#include "engines/js_exception.h"
#include "engines/js_traits.h"
#include "engines/quickjs/quickjs_engine_impl.h"
#include "engines/quickjs/qjs_value_raii.h"
#include "types/js_wrapper.h"

template <typename T_JS_VALUE>
class JsEngine;

template <>
class JsEngine<QjsValueRAII> {
public:
  inline static std::mutex instanceMutex;
  inline static bool isInitialized = false;

  std::unique_ptr<QuickJsEngineImpl> impl_{std::make_unique<QuickJsEngineImpl>()};

  JsEngine() { isInitialized = true; };

public:
  using T_JS_OBJECT = QjsValueRAII;
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
  [[nodiscard]] QjsValueRAII null() const { return QjsValueRAII(impl_->getContext(), JS_NULL); }
  [[nodiscard]] QjsValueRAII undefined() const {
    return QjsValueRAII(impl_->getContext(), JS_UNDEFINED);
  }

  [[nodiscard]] QjsValueRAII jsTrue() const { return QjsValueRAII(impl_->getContext(), JS_TRUE); }
  [[nodiscard]] QjsValueRAII jsFalse() const { return QjsValueRAII(impl_->getContext(), JS_FALSE); }

  [[nodiscard]] bool isArray(JSValueConst value) const { return JS_IsArray(value); }
  [[nodiscard]] bool isObject(JSValueConst value) const { return JS_IsObject(value); }
  [[nodiscard]] bool isBool(JSValueConst value) const { return JS_IsBool(value); }
  [[nodiscard]] bool isNull(JSValueConst value) const { return JS_IsNull(value); }
  [[nodiscard]] bool isUndefined(JSValueConst value) const { return JS_IsUndefined(value); }
  [[nodiscard]] bool isException(JSValueConst value) const { return JS_IsException(value); }

  [[nodiscard]] QjsValueRAII toObject(JSValueConst value) const {
    if (JS_IsObject(value) || JS_IsNull(value) || JS_IsUndefined(value)) {
      return QjsValueRAII(impl_->getContext(), JS_DupValue(impl_->getContext(), value));
    }
    return QjsValueRAII(impl_->getContext(), JS_ToObject(impl_->getContext(), value));
  }

  void setBaseFolderPath(const char* absolutePath) const { impl_->setBaseFolderPath(absolutePath); }
  // NOLINTEND(readability-convert-member-functions-to-static)

  [[nodiscard]] QjsValueRAII newArray() const {
    return QjsValueRAII(impl_->getContext(), JS_NewArray(impl_->getContext()));
  }

  [[nodiscard]] size_t getArrayLength(JSValueConst array) const {
    return impl_->getArrayLength(array);
  }

  void insertItemToArray(JSValueConst array, size_t index, JSValue value) const {
    impl_->insertItemToArray(array, index, JS_DupValue(impl_->getContext(), value));
  }

  [[nodiscard]] QjsValueRAII getArrayItem(JSValueConst array, const size_t index) const {
    return QjsValueRAII(impl_->getContext(), impl_->getArrayItem(array, index));
  }

  [[nodiscard]] QjsValueRAII newObject() const {
    return QjsValueRAII(impl_->getContext(), JS_NewObject(impl_->getContext()));
  }

  [[nodiscard]] QjsValueRAII getObjectProperty(JSValueConst obj, const char* propertyName) const {
    return QjsValueRAII(impl_->getContext(), impl_->getObjectProperty(obj, propertyName));
  }

  int setObjectProperty(JSValueConst obj, const char* propertyName, JSValue value) const {
    return impl_->setObjectProperty(obj, propertyName, JS_DupValue(impl_->getContext(), value));
  }

  using ExposeFunction = JSCFunction*;
  int setObjectFunction(JSValueConst obj,
                        const char* functionName,
                        const ExposeFunction cppFunction,
                        const int expectingArgc) const {
    return impl_->setObjectFunction(obj, functionName, cppFunction, expectingArgc);
  }

  [[nodiscard]] std::string toStdString(JSValueConst value) const {
    return impl_->toStdString(value);
  }

  [[nodiscard]] bool toBool(JSValueConst value) const {
    return JS_ToBool(impl_->getContext(), value) != 0;
  }

  [[nodiscard]] size_t toInt(JSValueConst value) const { return impl_->toInt(value); }

  [[nodiscard]] double toDouble(JSValueConst value) const { return impl_->toDouble(value); }

  [[nodiscard]] QjsValueRAII callFunction(JSValueConst func,
                                          JSValueConst thisArg,
                                          const int argc,
                                          QjsValueRAII* argv) const {
    std::vector<JSValue> args(argc);
    for (int i = 0; i < argc; ++i) {
      args[i] = argv[i];
    }
    return QjsValueRAII(impl_->getContext(),
                        impl_->callFunction(func, thisArg, argc, args.data()));
  }

  [[nodiscard]] QjsValueRAII newClassInstance(JSValueConst clazz,
                                              const int argc,
                                              QjsValueRAII* argv) const {
    std::vector<JSValue> args(argc);
    for (int i = 0; i < argc; ++i) {
      args[i] = argv[i];
    }
    return QjsValueRAII(impl_->getContext(), impl_->newClassInstance(clazz, argc, args.data()));
  }

  [[nodiscard]] QjsValueRAII getJsClassHavingMethod(JSValueConst module,
                                                    const char* methodName) const {
    return QjsValueRAII(impl_->getContext(), impl_->getJsClassHavingMethod(module, methodName));
  }

  [[nodiscard]] QjsValueRAII getMethodOfClassOrInstance(JSValueConst jsClass,
                                                        JSValueConst instance,
                                                        const char* methodName) const {
    return QjsValueRAII(impl_->getContext(),
                        impl_->getMethodOfClassOrInstance(jsClass, instance, methodName));
  }

  [[nodiscard]] bool isFunction(JSValueConst value) const {
    return JS_IsFunction(impl_->getContext(), value);
  }
  [[nodiscard]] QjsValueRAII getLatestException() const {
    return QjsValueRAII(impl_->getContext(), JS_GetException(impl_->getContext()));
  }

  void logErrorStackTrace(JSValueConst exception, const char* file, const int line) const {
    impl_->logErrorStackTrace(exception, file, line);
  }

  [[nodiscard]] QjsValueRAII duplicateValue(JSValueConst value) const {
    return QjsValueRAII(impl_->getContext(), JS_DupValue(impl_->getContext(), value));
  }

  template <typename... Args>
  void freeValue(const Args&... args) const {
    // RAII handles this
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
  [[nodiscard]] typename JsWrapper<T>::T_UNWRAP_TYPE unwrap(JSValueConst value) const {
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
  [[nodiscard]] std::enable_if_t<!is_shared_ptr_v<T>, QjsValueRAII> wrap(T* ptrValue) const {
    if (ptrValue == nullptr) {
      return QjsValueRAII(impl_->getContext(), JS_NULL);
    }
    using DereferencedType = std::decay_t<decltype(*ptrValue)>;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return QjsValueRAII(impl_->getContext(),
                        impl_->wrap(JsWrapper<DereferencedType>::typeName,
                                    const_cast<std::remove_const_t<DereferencedType>*>(
                                        reinterpret_cast<const DereferencedType*>(ptrValue)),
                                    "raw"));
  }

  template <typename T>
  [[nodiscard]] std::enable_if_t<is_shared_ptr_v<T>, QjsValueRAII> wrap(T ptrValue) const {
    if (ptrValue == nullptr) {
      return QjsValueRAII(impl_->getContext(), JS_NULL);
    }
    using Inner = shared_ptr_inner_t<decltype(ptrValue)>;
    auto ptr = std::make_unique<std::shared_ptr<Inner>>(ptrValue);
    return QjsValueRAII(impl_->getContext(),
                        impl_->wrap(JsWrapper<Inner>::typeName, ptr.release(), "shared"));
  }

  [[nodiscard]] QjsValueRAII wrap(const char* str) const {
    return QjsValueRAII(impl_->getContext(), impl_->toJsString(str));
  }
  [[nodiscard]] QjsValueRAII wrap(const std::string& str) const {
    return QjsValueRAII(impl_->getContext(), impl_->toJsString(str));
  }
  [[nodiscard]] QjsValueRAII wrap(const bool value) const {
    return QjsValueRAII(impl_->getContext(), JS_NewBool(impl_->getContext(), value));
  }
  [[nodiscard]] QjsValueRAII wrap(const size_t value) const {
    return QjsValueRAII(impl_->getContext(), impl_->toJsNumber(static_cast<int64_t>(value)));
  }
  [[nodiscard]] QjsValueRAII wrap(const int value) const {
    return QjsValueRAII(impl_->getContext(), impl_->toJsNumber(static_cast<int64_t>(value)));
  }
  [[nodiscard]] QjsValueRAII wrap(const double value) const {
    return QjsValueRAII(impl_->getContext(), impl_->toJsNumber(value));
  }

  [[nodiscard]] QjsValueRAII createInstanceOfModule(const char* moduleName,
                                                    std::vector<QjsValueRAII>& args,
                                                    const std::string& mainFuncName) const {
    std::vector<JSValue> qjsArgs(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
      qjsArgs[i] = args[i];
    }
    return QjsValueRAII(impl_->getContext(),
                        impl_->createInstanceOfModule(moduleName, qjsArgs, mainFuncName));
  }
  [[nodiscard]] QjsValueRAII loadJsFile(const char* fileName) const {
    return QjsValueRAII(impl_->getContext(), impl_->loadJsFile(fileName));
  }

  [[nodiscard]] QjsValueRAII eval(const char* code, const char* filename = "<eval>") const {
    return QjsValueRAII(impl_->getContext(), JS_Eval(impl_->getContext(), code, strlen(code),
                                                     filename, JS_EVAL_TYPE_GLOBAL));
  }

  [[nodiscard]] QjsValueRAII getGlobalObject() const {
    return QjsValueRAII(impl_->getContext(), JS_GetGlobalObject(impl_->getContext()));
  }

  [[nodiscard]] QjsValueRAII throwError(const JsErrorType errorType,
                                        const std::string& message) const {
    return QjsValueRAII(impl_->getContext(), impl_->throwError(errorType, message));
  }
};
