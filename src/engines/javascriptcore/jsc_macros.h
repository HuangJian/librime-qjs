#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include "engines/javascriptcore/javascriptcore_engine.h"  // IWYU pragma: export

#include <sstream>

// NOLINTBEGIN(cppcoreguidelines-macro-usage) function-like macro 'EXPORT_CLASS_IMPL' used; consider a 'constexpr' template function
#define DEFINE_GETTER(T_RIME_TYPE, propertyName, statement)                                    \
                                                                                               \
  DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertyName, statement);                                \
                                                                                               \
  static JSValueRef get_##propertyName##Jsc(JSContextRef ctx, JSObjectRef thisVal,             \
                                            JSStringRef functionName, JSValueRef* exception) { \
    auto& engine = JsEngine<JSValueRef>::instance();                                           \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                                      \
      return engine.wrap(statement);                                                           \
    }                                                                                          \
    return engine.undefined();                                                                 \
  }

#define DEFINE_SETTER(T_RIME_TYPE, jsName, assignment)                                           \
                                                                                                 \
  DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, assignment);                                       \
                                                                                                 \
  static bool set_##jsName##Jsc(JSContextRef ctx, JSObjectRef thisVal, JSStringRef propertyName, \
                                JSValueRef val, JSValueRef* exception) {                         \
    auto& engine = JsEngine<JSValueRef>::instance();                                             \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                                        \
      auto value = makeSetterValueProxy(engine, val);                                            \
      assignment;                                                                                \
      return true;                                                                               \
    }                                                                                            \
    std::stringstream msg;                                                                       \
    msg << "Failed to unwrap the js object to a cpp " << #T_RIME_TYPE << " object";              \
    *exception = JSValueMakeString(ctx, JscStringRAII(msg.str().c_str()));                       \
    return false;                                                                                \
  }

#define DEFINE_CFUNCTION(funcName, funcBody)                                                     \
                                                                                                 \
  DEFINE_CFUNCTION_QJS(funcName, funcBody);                                                      \
                                                                                                 \
  static JSValueRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, JSObjectRef thisVal,   \
                                  size_t argc, const JSValueRef argv[], JSValueRef* exception) { \
    auto& engine = JsEngine<JSValueRef>::instance();                                             \
    try {                                                                                        \
      funcBody;                                                                                  \
    } catch (const JsException& e) {                                                             \
      *exception = JSValueMakeString(ctx, JscStringRAII(e.what()));                              \
      return nullptr;                                                                            \
    }                                                                                            \
  }

#define DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements)                               \
                                                                                                 \
  DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements);                                \
                                                                                                 \
  static JSValueRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, JSObjectRef thisVal,   \
                                  size_t argc, const JSValueRef argv[], JSValueRef* exception) { \
    auto& engine = JsEngine<JSValueRef>::instance();                                             \
    if (argc < (expectingArgc)) {                                                                \
      std::stringstream msg;                                                                     \
      msg << #funcName << "(...) expects " << (expectingArgc) << " arguments";                   \
      *exception = JSValueMakeString(ctx, JscStringRAII(msg.str().c_str()));                     \
      return nullptr;                                                                            \
    }                                                                                            \
    try {                                                                                        \
      statements;                                                                                \
    } catch (const JsException& e) {                                                             \
      *exception = JSValueMakeString(ctx, JscStringRAII(e.what()));                              \
      return nullptr;                                                                            \
    }                                                                                            \
  }

#define EXPORT_CLASS_IMPL(className, block1, block2, block3, block4)               \
  EXPORT_CLASS_IMPL_QJS(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), \
                        EXPAND(block4));                                           \
  inline static JSClassRef classDefJsc = nullptr;

#define WITH_CONSTRUCTOR(funcName)                                                       \
  WITH_CONSTRUCTOR_QJS(funcName);                                                        \
  static JSObjectRef constructorJsc(JSContextRef ctx, JSObjectRef function, size_t argc, \
                                    const JSValueRef argv[], JSValueRef* exception) {    \
    auto val = funcName##Jsc(ctx, function, nullptr, argc, argv, exception);             \
    return JSValueToObject(ctx, val, nullptr);                                           \
  }

#define WITHOUT_CONSTRUCTOR \
  WITHOUT_CONSTRUCTOR_QJS;  \
  inline static JSObjectCallAsConstructorCallback constructorJsc = nullptr;

#define WITH_FINALIZER                                                     \
  WITH_FINALIZER_QJS;                                                      \
  static void finalizerJsc(JSObjectRef val) {                              \
    if (void* ptr = JSObjectGetPrivate(val)) {                             \
      if (auto* ppObj = static_cast<std::shared_ptr<T_RIME_TYPE>*>(ptr)) { \
        delete ppObj;                                                      \
        JSObjectSetPrivate(val, nullptr);                                  \
      }                                                                    \
    }                                                                      \
  };

#define WITHOUT_FINALIZER \
  WITHOUT_FINALIZER_QJS;  \
  inline static void (*finalizerJsc)(JSObjectRef) = nullptr;

#define DEFINE_PROPERTY_JSC_IMPL(name, cpp_name, enabled) \
  {#name, get_##name##Jsc, set_##name##Jsc, kJSPropertyAttributeNone},
#define DEFINE_PROPERTY_JSC(spec) DEFINE_PROPERTY_JSC_IMPL spec

#define WITH_PROPERTIES(...)        \
  WITH_PROPERTIES_QJS(__VA_ARGS__); \
  inline static JSStaticValue propertiesJsc[] = {FOR_EACH(DEFINE_PROPERTY_JSC, __VA_ARGS__)};

#define WITHOUT_PROPERTIES \
  WITHOUT_PROPERTIES_QJS;  \
  inline static JSStaticValue propertiesJsc[] = {};

#define DEFINE_GETTER_JSC_IMPL(name, payload, mode) \
  {#name, get_##name##Jsc, nullptr, kJSPropertyAttributeNone},
#define DEFINE_GETTER_JSC_EXPAND(spec) DEFINE_GETTER_JSC_IMPL spec
#define DEFINE_GETTER_JSC(spec) DEFINE_GETTER_JSC_EXPAND(NORMALIZE_GETTER_SPEC(spec))

#define WITH_GETTERS(...)       \
  WITH_GETTER_QJS(__VA_ARGS__); \
  inline static JSStaticValue gettersJsc[] = {FOR_EACH(DEFINE_GETTER_JSC, __VA_ARGS__)};

#define WITHOUT_GETTERS \
  WITHOUT_GETTER_QJS;   \
  inline static JSStaticValue gettersJsc[] = {};

#define DEFINE_FUNCTION_JSC(name) \
  {#name, name##Jsc, static_cast<JSPropertyAttributes>(name##_argc)},

#define WITH_FUNCTIONS(...)                                            \
  WITH_FUNCTIONS_QJS(__VA_ARGS__);                                     \
  inline static JSStaticFunction functionsJsc[] = {                    \
      FOR_EACH(DEFINE_FUNCTION_JSC, __VA_ARGS__){nullptr, nullptr, 0}, \
  };

#define WITHOUT_FUNCTIONS \
  WITHOUT_FUNCTIONS_QJS;  \
  inline static JSStaticFunction functionsJsc[] = {{nullptr, nullptr, 0}};
// NOLINTEND(cppcoreguidelines-macro-usage)
