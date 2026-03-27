#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include "engines/javascriptcore/javascriptcore_engine.h"  // IWYU pragma: export

#include <sstream>

// NOLINTBEGIN(cppcoreguidelines-macro-usage) function-like macro 'JS_PRIV_EXPORT_CLASS_IMPL' used; consider a 'constexpr' template function
#define JS_API_DEFINE_GETTER(T_RIME_TYPE, propertyName, statement)                             \
                                                                                               \
  JS_PRIV_QJS_DEFINE_GETTER_IMPL(T_RIME_TYPE, propertyName, statement);                        \
                                                                                               \
  static JSValueRef get_##propertyName##Jsc(JSContextRef ctx, JSObjectRef thisVal,             \
                                            JSStringRef functionName, JSValueRef* exception) { \
    auto& engine = JsEngine<JSValueRef>::instance();                                           \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                                      \
      return engine.wrap(statement);                                                           \
    }                                                                                          \
    return engine.undefined();                                                                 \
  }

#define JS_API_DEFINE_SETTER(T_RIME_TYPE, jsName, assignment)                                    \
                                                                                                 \
  JS_PRIV_QJS_DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, assignment);                               \
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

#define JS_API_DEFINE_CFUNCTION(funcName, funcBody)                                              \
                                                                                                 \
  JS_PRIV_QJS_DEFINE_CFUNCTION(funcName, funcBody);                                              \
                                                                                                 \
  static JSValueRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, JSObjectRef thisVal,   \
                                  size_t argc, const JSValueRef argv[], JSValueRef* exception) { \
    auto& engine = JsEngine<JSValueRef>::instance();                                             \
    [[maybe_unused]] auto obj = engine.unwrap<JsWrapperTypeT<JsWrapper>>(thisVal);               \
    try {                                                                                        \
      funcBody;                                                                                  \
    } catch (const JsException& e) {                                                             \
      *exception = JSValueMakeString(ctx, JscStringRAII(e.what()));                              \
      return nullptr;                                                                            \
    }                                                                                            \
  }

#define JS_API_DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements)                        \
                                                                                                 \
  JS_PRIV_QJS_DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements);                        \
                                                                                                 \
  static JSValueRef funcName##Jsc(JSContextRef ctx, JSObjectRef function, JSObjectRef thisVal,   \
                                  size_t argc, const JSValueRef argv[], JSValueRef* exception) { \
    if (argc < (expectingArgc)) {                                                                \
      std::stringstream msg;                                                                     \
      msg << #funcName << "(...) expects " << (expectingArgc) << " arguments";                   \
      *exception = JSValueMakeString(ctx, JscStringRAII(msg.str().c_str()));                     \
      return nullptr;                                                                            \
    }                                                                                            \
    auto& engine = JsEngine<JSValueRef>::instance();                                             \
    [[maybe_unused]] auto obj = engine.unwrap<JsWrapperTypeT<JsWrapper>>(thisVal);               \
    try {                                                                                        \
      statements;                                                                                \
    } catch (const JsException& e) {                                                             \
      *exception = JSValueMakeString(ctx, JscStringRAII(e.what()));                              \
      return nullptr;                                                                            \
    }                                                                                            \
  }

#define JS_PRIV_EXPORT_CLASS_IMPL(className, block1, block2, block3, block4)               \
  JS_PRIV_QJS_EXPORT_CLASS_IMPL(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), \
                                EXPAND(block4));                                           \
  inline static JSClassRef classDefJsc = nullptr;

#define JS_PRIV_WITH_CONSTRUCTOR_1(funcName)                                             \
  JS_PRIV_QJS_WITH_CONSTRUCTOR(funcName);                                                \
  static JSObjectRef constructorJsc(JSContextRef ctx, JSObjectRef function, size_t argc, \
                                    const JSValueRef argv[], JSValueRef* exception) {    \
    auto val = funcName##Jsc(ctx, function, nullptr, argc, argv, exception);             \
    return JSValueToObject(ctx, val, nullptr);                                           \
  }
#define JS_PRIV_WITH_CONSTRUCTOR_0() JS_PRIV_NO_CONSTRUCTOR
#define JS_PRIV_WITH_CONSTRUCTOR_N_IMPL(N, ...) JS_PRIV_WITH_CONSTRUCTOR_##N(__VA_ARGS__)
#define JS_PRIV_WITH_CONSTRUCTOR_N(N, ...) JS_PRIV_WITH_CONSTRUCTOR_N_IMPL(N, __VA_ARGS__)
#define JS_API_WITH_CONSTRUCTOR(...) \
  JS_PRIV_WITH_CONSTRUCTOR_N(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

#define JS_PRIV_NO_CONSTRUCTOR \
  JS_PRIV_QJS_NO_CONSTRUCTOR;  \
  inline static JSObjectCallAsConstructorCallback constructorJsc = nullptr;

#define JS_PRIV_WITH_FINALIZER                                             \
  JS_PRIV_QJS_WITH_FINALIZER;                                              \
  static void finalizerJsc(JSObjectRef val) {                              \
    if (void* ptr = JSObjectGetPrivate(val)) {                             \
      if (auto* ppObj = static_cast<std::shared_ptr<T_RIME_TYPE>*>(ptr)) { \
        delete ppObj;                                                      \
        JSObjectSetPrivate(val, nullptr);                                  \
      }                                                                    \
    }                                                                      \
  };

#define JS_PRIV_NO_FINALIZER \
  JS_PRIV_QJS_NO_FINALIZER;  \
  inline static void (*finalizerJsc)(JSObjectRef) = nullptr;

#define JS_PRIV_JSC_DEFINE_PROPERTY_IMPL(name, cpp_name, enabled) \
  {#name, get_##name##Jsc, set_##name##Jsc, kJSPropertyAttributeNone},
#define JS_PRIV_JSC_DEFINE_PROPERTY(spec) JS_PRIV_JSC_DEFINE_PROPERTY_IMPL spec

#define JS_API_WITH_PROPERTIES(...)               \
  JS_PRIV_QJS_WITH_PROPERTIES(__VA_ARGS__);       \
  inline static JSStaticValue propertiesJsc[] = { \
      FOR_EACH(JS_PRIV_JSC_DEFINE_PROPERTY, __VA_ARGS__)};

#define JS_PRIV_JSC_DEFINE_GETTER_IMPL(name, payload, mode) \
  {#name, get_##name##Jsc, nullptr, kJSPropertyAttributeNone},
#define JS_PRIV_JSC_DEFINE_GETTER_EXPAND(spec) JS_PRIV_JSC_DEFINE_GETTER_IMPL spec
#define JS_PRIV_JSC_DEFINE_GETTER(spec) \
  JS_PRIV_JSC_DEFINE_GETTER_EXPAND(JS_PRIV_NORMALIZE_GETTER_SPEC(spec))

#define JS_API_WITH_GETTERS(...)         \
  JS_PRIV_QJS_WITH_GETTERS(__VA_ARGS__); \
  inline static JSStaticValue gettersJsc[] = {FOR_EACH(JS_PRIV_JSC_DEFINE_GETTER, __VA_ARGS__)};

#define JS_PRIV_JSC_DEFINE_FUNCTION(name) \
  {#name, name##Jsc, static_cast<JSPropertyAttributes>(name##_argc)},

#define JS_API_WITH_FUNCTIONS(...)                                             \
  JS_PRIV_QJS_WITH_FUNCTIONS(__VA_ARGS__);                                     \
  inline static JSStaticFunction functionsJsc[] = {                            \
      FOR_EACH(JS_PRIV_JSC_DEFINE_FUNCTION, __VA_ARGS__){nullptr, nullptr, 0}, \
  };
// NOLINTEND(cppcoreguidelines-macro-usage)
