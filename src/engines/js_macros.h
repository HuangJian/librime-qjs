#pragma once

#include <quickjs.h>

#include <string>
#include <type_traits>
#include <utility>

#include "engines/for_each_macros.h"
#include "engines/quickjs/quickjs_engine.h"  // IWYU pragma: export

template <typename T_JS_VALUE>
class JsSetterValueProxy {
  const JsEngine<T_JS_VALUE>& engine_;
  const T_JS_VALUE& jsValue_;

public:
  JsSetterValueProxy(const JsEngine<T_JS_VALUE>& engine, const T_JS_VALUE& jsValue)
      : engine_(engine), jsValue_(jsValue) {}

  template <typename T, std::enable_if_t<std::is_same_v<std::decay_t<T>, bool>, int> = 0>
  operator T() const {
    return static_cast<T>(engine_.toBool(jsValue_));
  }

  template <typename T, std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>, int> = 0>
  operator T() const {
    return engine_.toStdString(jsValue_);
  }

  template <typename T, std::enable_if_t<std::is_floating_point_v<std::decay_t<T>>, int> = 0>
  operator T() const {
    return static_cast<T>(engine_.toDouble(jsValue_));
  }

  template <typename T,
            std::enable_if_t<std::is_integral_v<std::decay_t<T>> &&
                                 !std::is_same_v<std::decay_t<T>, bool> &&
                                 !std::is_same_v<std::decay_t<T>, char> &&
                                 !std::is_same_v<std::decay_t<T>, signed char> &&
                                 !std::is_same_v<std::decay_t<T>, unsigned char>,
                             int> = 0>
  operator T() const {
    return static_cast<T>(engine_.toInt(jsValue_));
  }

  template <typename T, std::enable_if_t<std::is_enum_v<std::decay_t<T>>, int> = 0>
  operator T() const {
    using Underlying = std::underlying_type_t<std::decay_t<T>>;
    return static_cast<T>(static_cast<Underlying>(engine_.toInt(jsValue_)));
  }
};

template <typename T_JS_VALUE>
JsSetterValueProxy<T_JS_VALUE> makeSetterValueProxy(const JsEngine<T_JS_VALUE>& engine,
                                                    const T_JS_VALUE& jsValue) {
  return JsSetterValueProxy<T_JS_VALUE>(engine, jsValue);
}

#ifdef _ENABLE_JAVASCRIPTCORE
#include "engines/javascriptcore/jsc_macros.h"
#else
#define DEFINE_GETTER(T_RIME_TYPE, propertyName, statement) \
  DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertyName, statement)

#define DEFINE_SETTER(T_RIME_TYPE, jsName, assignment) \
  DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, assignment)

#define DEFINE_CFUNCTION(funcName, funcBody) DEFINE_CFUNCTION_QJS(funcName, funcBody)

#define DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements) \
  DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)

#define EXPORT_CLASS_IMPL(className, block1, block2, block3, block4) \
  EXPORT_CLASS_IMPL_QJS(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), EXPAND(block4));

#define WITH_CONSTRUCTOR(funcName) WITH_CONSTRUCTOR_QJS(funcName)
#define WITHOUT_CONSTRUCTOR WITHOUT_CONSTRUCTOR_QJS

#define WITH_FINALIZER WITH_FINALIZER_QJS
#define WITHOUT_FINALIZER WITHOUT_FINALIZER_QJS

#define WITH_PROPERTIES(...) WITH_PROPERTIES_QJS(__VA_ARGS__)
#define WITHOUT_PROPERTIES WITHOUT_PROPERTIES_QJS

#define WITH_GETTERS(...) WITH_GETTER_QJS(__VA_ARGS__)
#define WITHOUT_GETTERS WITHOUT_GETTER_QJS

#define WITH_FUNCTIONS(...) WITH_FUNCTIONS_QJS(__VA_ARGS__)
#define WITHOUT_FUNCTIONS WITHOUT_FUNCTIONS_QJS
#endif

template <typename T, std::size_t N>
constexpr std::size_t countof(const T (& /*unused*/)[N]) noexcept {
  return N;
}

// Property specs used by WITH_PROPERTIES(...).
// CUSTOM_PROPERTY(name): use pre-defined get_/set_ accessors.
// AUTO_PROPERTY(name): auto-generate get_/set_ accessors using obj->name() and obj->set_name(value).
#define CUSTOM_PROPERTY(name) (name, name, 0)

#define AUTO_PROPERTY_1(name) (name, name, 1)
#define AUTO_PROPERTY_2(name, cpp_name) (name, cpp_name, 1)
#define AUTO_PROPERTY_CHOOSER(_1, _2, NAME, ...) NAME
#define AUTO_PROPERTY(...) \
  EXPAND(AUTO_PROPERTY_CHOOSER(__VA_ARGS__, AUTO_PROPERTY_2, AUTO_PROPERTY_1)(__VA_ARGS__))

#define CUSTOM_PROPERTIES(...) FOR_EACH_COMMA(CUSTOM_PROPERTY, __VA_ARGS__)
#define AUTO_PROPERTIES(...) FOR_EACH_COMMA(AUTO_PROPERTY, __VA_ARGS__)
#define AUTO_PROPERTIES_RENAMED(...) FOR_EACH_PAIR_COMMA(AUTO_PROPERTY, __VA_ARGS__)

// NOLINTBEGIN(cppcoreguidelines-macro-usage) function-like macro 'DEFINE_GETTER' used; consider a 'constexpr' template function
// =============== GETTER ===============
#define DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertyName, statement)        \
  static JSValue get_##propertyName(JSContext* ctx, JSValueConst thisVal) { \
    auto& engine = JsEngine<JSValue>::instance();                           \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                   \
      return engine.wrap(statement);                                        \
    }                                                                       \
    return JS_UNDEFINED;                                                    \
  }

// =============== SETTER ===============
#define DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, assignment)                    \
  static JSValue set_##jsName(JSContext* ctx, JSValueConst thisVal, JSValue val) { \
    auto& engine = JsEngine<JSValue>::instance();                                  \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                          \
      auto value = makeSetterValueProxy(engine, val);                              \
      assignment;                                                                  \
      return JS_UNDEFINED;                                                         \
    }                                                                              \
    auto* format = "Failed to unwrap the js object to a cpp %s object";            \
    return JS_ThrowTypeError(ctx, format, #T_RIME_TYPE);                           \
  }

// =============== FUNCTION ===============

#define DEFINE_CFUNCTION_QJS(funcName, funcBody)                                      \
  static constexpr int funcName##_argc = 0;                                           \
  static JSValue funcName(JSContext* ctx, JSValue thisVal, int argc, JSValue* argv) { \
    auto& engine = JsEngine<JSValue>::instance();                                     \
    try {                                                                             \
      funcBody;                                                                       \
    } catch (const JsException& e) {                                                  \
      return JS_ThrowTypeError(ctx, "%s", e.what());                                  \
    }                                                                                 \
  }

#define DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)                           \
  static constexpr int funcName##_argc = expectingArgc;                                          \
  static JSValue funcName(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {  \
    if (argc < (expectingArgc)) {                                                                \
      return JS_ThrowSyntaxError(ctx, "%s(...) expects %d arguments", #funcName, expectingArgc); \
    }                                                                                            \
    auto& engine = JsEngine<JSValue>::instance();                                                \
    try {                                                                                        \
      statements;                                                                                \
    } catch (const JsException& e) {                                                             \
      return JS_ThrowTypeError(ctx, "%s", e.what());                                             \
    }                                                                                            \
  }

// =============== QJS CLASS DEFINITION ===============
#define EXPORT_CLASS_IMPL_QJS(className, block1, block2, block3, block4) \
                                                                         \
  using T_RIME_TYPE = className;                                         \
                                                                         \
  inline static const char* typeName = #className;                       \
                                                                         \
  inline static JSClassID jsClassId = 0;                                 \
                                                                         \
  inline static JSClassDef JS_CLASS_DEF = {                              \
      .class_name = #className,                                          \
      .finalizer = nullptr,                                              \
      .gc_mark = nullptr,                                                \
      .call = nullptr,                                                   \
      .exotic = nullptr,                                                 \
  };                                                                     \
                                                                         \
  block1;                                                                \
  block2;                                                                \
  block3;                                                                \
  block4;

#define EXPORT_CLASS_WITH_RAW_POINTER(className, block1, block2, block3, block4)                \
  using T_UNWRAP_TYPE = raw_ptr_type<className>::type;                                          \
  EXPORT_CLASS_IMPL(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), EXPAND(block4)); \
  WITHOUT_FINALIZER;  // the attached raw pointer is passed from Rime, should not free it in qjs

#define EXPORT_CLASS_WITH_SHARED_POINTER(className, block1, block2, block3, block4)             \
  using T_UNWRAP_TYPE = std::shared_ptr<className>;                                             \
  EXPORT_CLASS_IMPL(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), EXPAND(block4)); \
  WITH_FINALIZER;  // the attached shared pointer's reference count should be decremented when the js object is freed

#define WITH_CONSTRUCTOR_QJS(funcName)                  \
  inline static JSCFunction* constructorQjs = funcName; \
  inline static const int CONSTRUCTOR_ARGC = funcName##_argc;
#define WITHOUT_CONSTRUCTOR_QJS                        \
  inline static JSCFunction* constructorQjs = nullptr; \
  inline static const int CONSTRUCTOR_ARGC = 0;

#define WITH_FINALIZER_QJS                                                        \
  inline static JSClassFinalizer* finalizerQjs = [](JSRuntime* rt, JSValue val) { \
    if (void* ptr = JS_GetOpaque(val, jsClassId)) {                               \
      if (auto* ppObj = static_cast<std::shared_ptr<T_RIME_TYPE>*>(ptr)) {        \
        delete ppObj;                                                             \
        JS_SetOpaque(val, nullptr);                                               \
      }                                                                           \
    }                                                                             \
  };
#define WITHOUT_FINALIZER_QJS inline static JSClassFinalizer* finalizerQjs = nullptr;

#define DEFINE_AUTO_PROPERTY_ACCESSOR_0(name, cpp_name)
#define DEFINE_AUTO_PROPERTY_ACCESSOR_1(name, cpp_name)                                \
  template <typename T_OBJ>                                                            \
  static auto get_auto_property_##name(T_OBJ&& obj, int)->decltype(obj->cpp_name()) {  \
    return obj->cpp_name();                                                            \
  }                                                                                    \
  template <typename T_OBJ>                                                            \
  static auto get_auto_property_##name(T_OBJ&& obj, long)->decltype((obj->cpp_name)) { \
    return obj->cpp_name;                                                              \
  }                                                                                    \
  template <typename T_OBJ, typename T_VALUE>                                          \
  static auto set_auto_property_##name(T_OBJ&& obj, T_VALUE&& value, int)              \
      ->decltype(obj->set_##cpp_name(std::forward<T_VALUE>(value)), void()) {          \
    obj->set_##cpp_name(std::forward<T_VALUE>(value));                                 \
  }                                                                                    \
  template <typename T_OBJ, typename T_VALUE>                                          \
  static auto set_auto_property_##name(T_OBJ&& obj, T_VALUE&& value, long)             \
      ->decltype((obj->cpp_name = std::forward<T_VALUE>(value)), void()) {             \
    obj->cpp_name = std::forward<T_VALUE>(value);                                      \
  }                                                                                    \
  DEFINE_GETTER(T_RIME_TYPE, name, get_auto_property_##name(obj, 0))                   \
  DEFINE_SETTER(T_RIME_TYPE, name, set_auto_property_##name(obj, value, 0))

#define DEFINE_AUTO_PROPERTY_ACCESSOR_IMPL(name, cpp_name, enabled) \
  DEFINE_AUTO_PROPERTY_ACCESSOR_##enabled(name, cpp_name)
#define DEFINE_AUTO_PROPERTY_ACCESSOR(spec) DEFINE_AUTO_PROPERTY_ACCESSOR_IMPL spec

#define DEFINE_PROPERTY_ENTRY_IMPL(name, cpp_name, enabled) \
  JS_CGETSET_DEF(#name, get_##name, set_##name),
#define DEFINE_PROPERTY_ENTRY(spec) DEFINE_PROPERTY_ENTRY_IMPL spec

#define WITH_PROPERTIES_QJS(...)                                \
  FOR_EACH(DEFINE_AUTO_PROPERTY_ACCESSOR, __VA_ARGS__)          \
  inline static const JSCFunctionListEntry PROPERTIES_QJS[] = { \
      FOR_EACH(DEFINE_PROPERTY_ENTRY, __VA_ARGS__)};            \
  inline static const size_t PROPERTIES_SIZE = sizeof(PROPERTIES_QJS) / sizeof(PROPERTIES_QJS[0]);
#define WITHOUT_PROPERTIES_QJS                                    \
  inline static const JSCFunctionListEntry PROPERTIES_QJS[] = {}; \
  inline static const size_t PROPERTIES_SIZE = 0;

#define DEFINE_GETTER_QJS(name) JS_CGETSET_DEF(#name, get_##name, nullptr),

#define WITH_GETTER_QJS(...)                                 \
  inline static const JSCFunctionListEntry GETTERS_QJS[] = { \
      FOR_EACH(DEFINE_GETTER_QJS, __VA_ARGS__)};             \
  inline static const size_t GETTERS_SIZE = sizeof(GETTERS_QJS) / sizeof(GETTERS_QJS[0]);
#define WITHOUT_GETTER_QJS                                     \
  inline static const JSCFunctionListEntry GETTERS_QJS[] = {}; \
  inline static const size_t GETTERS_SIZE = 0;

#define DEFINE_FUNCTION_QJS(name) JS_CFUNC_DEF(#name, static_cast<uint8_t>(name##_argc), name),

#define WITH_FUNCTIONS_QJS(...)                                \
  inline static const JSCFunctionListEntry FUNCTIONS_QJS[] = { \
      FOR_EACH(DEFINE_FUNCTION_QJS, __VA_ARGS__)};             \
  inline static const size_t FUNCTIONS_SIZE = sizeof(FUNCTIONS_QJS) / sizeof(FUNCTIONS_QJS[0]);
#define WITHOUT_FUNCTIONS_QJS                                    \
  inline static const JSCFunctionListEntry FUNCTIONS_QJS[] = {}; \
  inline static const size_t FUNCTIONS_SIZE = 0;
// NOLINTEND(cppcoreguidelines-macro-usage)
