#pragma once

#include <quickjs.h>

#include <string>
#include <type_traits>

#include "engines/for_each_macros.h"
#include "engines/quickjs/quickjs_engine.h"  // IWYU pragma: export

template <typename T_RIME_TYPE>
class JsWrapper;

template <typename T_WRAPPER>
struct JsWrapperType;

template <typename T_RIME_TYPE>
struct JsWrapperType<JsWrapper<T_RIME_TYPE>> {
  using type = T_RIME_TYPE;
};

template <typename T_WRAPPER>
using JsWrapperTypeT = typename JsWrapperType<T_WRAPPER>::type;

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
#define JS_API_DEFINE_GETTER(T_RIME_TYPE, propertyName, statement) \
  JS_PRIV_QJS_DEFINE_GETTER_IMPL(T_RIME_TYPE, propertyName, statement)

#define JS_API_DEFINE_SETTER(T_RIME_TYPE, jsName, assignment) \
  JS_PRIV_QJS_DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, assignment)

#define JS_API_DEFINE_CFUNCTION(funcName, funcBody) JS_PRIV_QJS_DEFINE_CFUNCTION(funcName, funcBody)

#define JS_API_DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements) \
  JS_PRIV_QJS_DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements)

#define JS_PRIV_EXPORT_CLASS_IMPL(className, block1, block2, block3, block4)               \
  JS_PRIV_QJS_EXPORT_CLASS_IMPL(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), \
                                EXPAND(block4));

#define JS_PRIV_WITH_CONSTRUCTOR_0() JS_PRIV_QJS_NO_CONSTRUCTOR
#define JS_PRIV_WITH_CONSTRUCTOR_1(funcName) JS_PRIV_QJS_WITH_CONSTRUCTOR(funcName)
#define JS_PRIV_WITH_CONSTRUCTOR_N_IMPL(N, ...) JS_PRIV_WITH_CONSTRUCTOR_##N(__VA_ARGS__)
#define JS_PRIV_WITH_CONSTRUCTOR_N(N, ...) JS_PRIV_WITH_CONSTRUCTOR_N_IMPL(N, __VA_ARGS__)
#define JS_API_WITH_CONSTRUCTOR(...) \
  JS_PRIV_WITH_CONSTRUCTOR_N(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

#define JS_PRIV_WITH_FINALIZER JS_PRIV_QJS_WITH_FINALIZER
#define JS_PRIV_NO_FINALIZER JS_PRIV_QJS_NO_FINALIZER

#define JS_API_WITH_PROPERTIES(...) JS_PRIV_QJS_WITH_PROPERTIES(__VA_ARGS__)

#define JS_API_WITH_GETTERS(...) JS_PRIV_QJS_WITH_GETTERS(__VA_ARGS__)

#define JS_API_WITH_FUNCTIONS(...) JS_PRIV_QJS_WITH_FUNCTIONS(__VA_ARGS__)
#endif

template <typename T, std::size_t N>
constexpr std::size_t countof(const T (& /*unused*/)[N]) noexcept {
  return N;
}

// Property specs used by JS_API_WITH_PROPERTIES(...).
// JS_API_CUSTOM_PROPERTIES(...): use pre-defined get_/set_ accessors.
// JS_API_AUTO_PROPERTY(name): auto-generate get_/set_ accessors using obj->name() and obj->set_name(value).
#define JS_PRIV_CUSTOM_PROPERTY(name) (name, name, 0)

#define JS_PRIV_AUTO_PROPERTY_1(name) (name, name, 1)
#define JS_PRIV_AUTO_PROPERTY_2(name, cpp_name) (name, cpp_name, 1)
#define JS_PRIV_AUTO_PROPERTY_CHOOSER(_1, _2, NAME, ...) NAME
#define JS_API_AUTO_PROPERTY(...)                                            \
  EXPAND(JS_PRIV_AUTO_PROPERTY_CHOOSER(__VA_ARGS__, JS_PRIV_AUTO_PROPERTY_2, \
                                       JS_PRIV_AUTO_PROPERTY_1)(__VA_ARGS__))

#define JS_PRIV_PP_CAT_IMPL(a, b) a##b
#define JS_PRIV_PP_CAT(a, b) JS_PRIV_PP_CAT_IMPL(a, b)
#define JS_PRIV_PP_CHECK_N(x, n, ...) n
#define JS_PRIV_PP_CHECK(...) JS_PRIV_PP_CHECK_N(__VA_ARGS__, 0, )
#define JS_PRIV_PP_PROBE(x) x, 1,
#define JS_PRIV_PP_IS_PAREN_PROBE(...) JS_PRIV_PP_PROBE(~)
#define JS_PRIV_PP_IS_PAREN(x) JS_PRIV_PP_CHECK(JS_PRIV_PP_IS_PAREN_PROBE x)

#define JS_PRIV_AUTO_PROPERTY_ITEM_PLAIN(x) JS_API_AUTO_PROPERTY(x)
#define JS_PRIV_AUTO_PROPERTY_ITEM_RENAMED(js_name, cpp_name) \
  JS_API_AUTO_PROPERTY(js_name, cpp_name)
#define JS_PRIV_AUTO_PROPERTY_ITEM_IMPL_0(x) JS_PRIV_AUTO_PROPERTY_ITEM_PLAIN(x)
#define JS_PRIV_AUTO_PROPERTY_ITEM_IMPL_1(x) JS_PRIV_AUTO_PROPERTY_ITEM_RENAMED x
#define JS_PRIV_AUTO_PROPERTY_ITEM(x) \
  JS_PRIV_PP_CAT(JS_PRIV_AUTO_PROPERTY_ITEM_IMPL_, JS_PRIV_PP_IS_PAREN(x))(x)

#define JS_API_CUSTOM_PROPERTIES(...) FOR_EACH_COMMA(JS_PRIV_CUSTOM_PROPERTY, __VA_ARGS__)
#define JS_API_AUTO_PROPERTIES(...) FOR_EACH_COMMA(JS_PRIV_AUTO_PROPERTY_ITEM, __VA_ARGS__)

// NOLINTBEGIN(cppcoreguidelines-macro-usage) function-like macro 'JS_API_DEFINE_GETTER' used; consider a 'constexpr' template function
// =============== GETTER ===============
#define JS_PRIV_QJS_DEFINE_GETTER_IMPL(T_RIME_TYPE, propertyName, statement) \
  static JSValue get_##propertyName(JSContext* ctx, JSValueConst thisVal) {  \
    auto& engine = JsEngine<JSValue>::instance();                            \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                    \
      return engine.wrap(statement);                                         \
    }                                                                        \
    return JS_UNDEFINED;                                                     \
  }

// =============== SETTER ===============
#define JS_PRIV_QJS_DEFINE_SETTER_IMPL(T_RIME_TYPE, jsName, assignment)            \
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

#define JS_PRIV_QJS_DEFINE_CFUNCTION(funcName, funcBody)                              \
  static constexpr int funcName##_argc = 0;                                           \
  static JSValue funcName(JSContext* ctx, JSValue thisVal, int argc, JSValue* argv) { \
    auto& engine = JsEngine<JSValue>::instance();                                     \
    [[maybe_unused]] auto obj = engine.unwrap<JsWrapperTypeT<JsWrapper>>(thisVal);    \
    try {                                                                             \
      funcBody;                                                                       \
    } catch (const JsException& e) {                                                  \
      return JS_ThrowTypeError(ctx, "%s", e.what());                                  \
    }                                                                                 \
  }

#define JS_PRIV_QJS_DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements)                   \
  static constexpr int funcName##_argc = expectingArgc;                                          \
  static JSValue funcName(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {  \
    if (argc < (expectingArgc)) {                                                                \
      return JS_ThrowSyntaxError(ctx, "%s(...) expects %d arguments", #funcName, expectingArgc); \
    }                                                                                            \
    auto& engine = JsEngine<JSValue>::instance();                                                \
    [[maybe_unused]] auto obj = engine.unwrap<JsWrapperTypeT<JsWrapper>>(thisVal);               \
    try {                                                                                        \
      statements;                                                                                \
    } catch (const JsException& e) {                                                             \
      return JS_ThrowTypeError(ctx, "%s", e.what());                                             \
    }                                                                                            \
  }

// =============== QJS CLASS DEFINITION ===============
#define JS_PRIV_QJS_EXPORT_CLASS_IMPL(className, block1, block2, block3, block4) \
                                                                                 \
  using T_RIME_TYPE = className;                                                 \
                                                                                 \
  inline static const char* typeName = #className;                               \
                                                                                 \
  inline static JSClassID jsClassId = 0;                                         \
                                                                                 \
  inline static JSClassDef JS_CLASS_DEF = {                                      \
      .class_name = #className,                                                  \
      .finalizer = nullptr,                                                      \
      .gc_mark = nullptr,                                                        \
      .call = nullptr,                                                           \
      .exotic = nullptr,                                                         \
  };                                                                             \
                                                                                 \
  block1;                                                                        \
  block2;                                                                        \
  block3;                                                                        \
  block4;

#define JS_API_EXPORT_CLASS_WITH_RAW_POINTER(className, block1, block2, block3, block4) \
  using T_UNWRAP_TYPE = raw_ptr_type<className>::type;                                  \
  JS_PRIV_EXPORT_CLASS_IMPL(className, EXPAND(block1), EXPAND(block2), EXPAND(block3),  \
                            EXPAND(block4));                                            \
  JS_PRIV_NO_FINALIZER;  // the attached raw pointer is passed from Rime, should not free it in qjs

#define JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(className, block1, block2, block3, block4) \
  using T_UNWRAP_TYPE = std::shared_ptr<className>;                                        \
  JS_PRIV_EXPORT_CLASS_IMPL(className, EXPAND(block1), EXPAND(block2), EXPAND(block3),     \
                            EXPAND(block4));                                               \
  JS_PRIV_WITH_FINALIZER;  // the attached shared pointer's reference count should be decremented when the js object is freed

#define JS_PRIV_QJS_WITH_CONSTRUCTOR(funcName)          \
  inline static JSCFunction* constructorQjs = funcName; \
  inline static const int CONSTRUCTOR_ARGC = funcName##_argc;
#define JS_PRIV_QJS_NO_CONSTRUCTOR                     \
  inline static JSCFunction* constructorQjs = nullptr; \
  inline static const int CONSTRUCTOR_ARGC = 0;

#define JS_PRIV_QJS_WITH_FINALIZER                                                \
  inline static JSClassFinalizer* finalizerQjs = [](JSRuntime* rt, JSValue val) { \
    if (void* ptr = JS_GetOpaque(val, jsClassId)) {                               \
      if (auto* ppObj = static_cast<std::shared_ptr<T_RIME_TYPE>*>(ptr)) {        \
        delete ppObj;                                                             \
        JS_SetOpaque(val, nullptr);                                               \
      }                                                                           \
    }                                                                             \
  };
#define JS_PRIV_QJS_NO_FINALIZER inline static JSClassFinalizer* finalizerQjs = nullptr;

#define JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR_0(name, cpp_name)
#define JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR_1(name, cpp_name)                        \
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
  JS_API_DEFINE_GETTER(T_RIME_TYPE, name, get_auto_property_##name(obj, 0))            \
  JS_API_DEFINE_SETTER(T_RIME_TYPE, name, set_auto_property_##name(obj, value, 0))

#define JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR_IMPL(name, cpp_name, enabled) \
  JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR_##enabled(name, cpp_name)
#define JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR(spec) JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR_IMPL spec

#define JS_PRIV_DEFINE_PROPERTY_ENTRY_IMPL(name, cpp_name, enabled) \
  JS_CGETSET_DEF(#name, get_##name, set_##name),
#define JS_PRIV_DEFINE_PROPERTY_ENTRY(spec) JS_PRIV_DEFINE_PROPERTY_ENTRY_IMPL spec

#define JS_PRIV_QJS_WITH_PROPERTIES(...)                        \
  FOR_EACH(JS_PRIV_DEFINE_AUTO_PROPERTY_ACCESSOR, __VA_ARGS__)  \
  inline static const JSCFunctionListEntry PROPERTIES_QJS[] = { \
      FOR_EACH(JS_PRIV_DEFINE_PROPERTY_ENTRY, __VA_ARGS__)};    \
  inline static const size_t PROPERTIES_SIZE = sizeof(PROPERTIES_QJS) / sizeof(PROPERTIES_QJS[0]);
#define JS_PRIV_DEFINE_GETTER_ACCESSOR_manual(name, payload)
#define JS_PRIV_DEFINE_GETTER_ACCESSOR_custom(name, statement) \
  JS_API_DEFINE_GETTER(T_RIME_TYPE, name, statement)
#define JS_PRIV_DEFINE_GETTER_ACCESSOR_auto(name, cpp_name)                          \
  template <typename T_OBJ>                                                          \
  static auto get_auto_getter_##name(T_OBJ&& obj, int)->decltype(obj->cpp_name()) {  \
    return obj->cpp_name();                                                          \
  }                                                                                  \
  template <typename T_OBJ>                                                          \
  static auto get_auto_getter_##name(T_OBJ&& obj, long)->decltype((obj->cpp_name)) { \
    return obj->cpp_name;                                                            \
  }                                                                                  \
  JS_API_DEFINE_GETTER(T_RIME_TYPE, name, get_auto_getter_##name(obj, 0))

#define JS_PRIV_NORMALIZE_GETTER_SPEC_PLAIN(x) (x, x, auto)
#define JS_PRIV_NORMALIZE_GETTER_SPEC_2(name, statement) (name, statement, custom)
#define JS_PRIV_NORMALIZE_GETTER_SPEC_3(name, payload, mode) (name, payload, mode)
#define JS_PRIV_NORMALIZE_GETTER_SPEC_TUPLE_CHOOSER(_1, _2, _3, NAME, ...) NAME
#define JS_PRIV_NORMALIZE_GETTER_SPEC_TUPLE_IMPL(...) \
  EXPAND(JS_PRIV_NORMALIZE_GETTER_SPEC_TUPLE_CHOOSER( \
      __VA_ARGS__, JS_PRIV_NORMALIZE_GETTER_SPEC_3, JS_PRIV_NORMALIZE_GETTER_SPEC_2)(__VA_ARGS__))
#define JS_PRIV_NORMALIZE_GETTER_SPEC_TUPLE(x) JS_PRIV_NORMALIZE_GETTER_SPEC_TUPLE_IMPL x
#define JS_PRIV_NORMALIZE_GETTER_SPEC_IMPL_0(x) JS_PRIV_NORMALIZE_GETTER_SPEC_PLAIN(x)
#define JS_PRIV_NORMALIZE_GETTER_SPEC_IMPL_1(x) JS_PRIV_NORMALIZE_GETTER_SPEC_TUPLE(x)
#define JS_PRIV_NORMALIZE_GETTER_SPEC(x) \
  JS_PRIV_PP_CAT(JS_PRIV_NORMALIZE_GETTER_SPEC_IMPL_, JS_PRIV_PP_IS_PAREN(x))(x)

#define JS_PRIV_DEFINE_GETTER_ACCESSOR_IMPL(name, payload, mode) \
  JS_PRIV_DEFINE_GETTER_ACCESSOR_##mode(name, payload)
#define JS_PRIV_DEFINE_GETTER_ACCESSOR_EXPAND(spec) JS_PRIV_DEFINE_GETTER_ACCESSOR_IMPL spec
#define JS_PRIV_DEFINE_GETTER_ACCESSOR_FROM_SPEC(spec) \
  JS_PRIV_DEFINE_GETTER_ACCESSOR_EXPAND(JS_PRIV_NORMALIZE_GETTER_SPEC(spec))

#define JS_PRIV_DEFINE_GETTER_ENTRY_IMPL(name, payload, mode) \
  JS_CGETSET_DEF(#name, get_##name, nullptr),
#define JS_PRIV_DEFINE_GETTER_ENTRY_EXPAND(spec) JS_PRIV_DEFINE_GETTER_ENTRY_IMPL spec
#define JS_PRIV_QJS_DEFINE_GETTER(spec) \
  JS_PRIV_DEFINE_GETTER_ENTRY_EXPAND(JS_PRIV_NORMALIZE_GETTER_SPEC(spec))

#define JS_PRIV_QJS_WITH_GETTERS(...)                             \
  FOR_EACH(JS_PRIV_DEFINE_GETTER_ACCESSOR_FROM_SPEC, __VA_ARGS__) \
  inline static const JSCFunctionListEntry GETTERS_QJS[] = {      \
      FOR_EACH(JS_PRIV_QJS_DEFINE_GETTER, __VA_ARGS__)};          \
  inline static const size_t GETTERS_SIZE = sizeof(GETTERS_QJS) / sizeof(GETTERS_QJS[0]);
#define JS_PRIV_QJS_DEFINE_FUNCTION(name) \
  JS_CFUNC_DEF(#name, static_cast<uint8_t>(name##_argc), name),

#define JS_PRIV_QJS_WITH_FUNCTIONS(...)                        \
  inline static const JSCFunctionListEntry FUNCTIONS_QJS[] = { \
      FOR_EACH(JS_PRIV_QJS_DEFINE_FUNCTION, __VA_ARGS__)};     \
  inline static const size_t FUNCTIONS_SIZE = sizeof(FUNCTIONS_QJS) / sizeof(FUNCTIONS_QJS[0]);
// NOLINTEND(cppcoreguidelines-macro-usage)
