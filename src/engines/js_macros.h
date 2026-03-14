#pragma once

#include <quickjs.h>

#include "engines/for_each_macros.h"
#include "engines/quickjs/quickjs_engine.h"  // IWYU pragma: export

#ifdef _ENABLE_JAVASCRIPTCORE
#include "engines/javascriptcore/jsc_macros.h"
#else
#define DEFINE_GETTER(T_RIME_TYPE, propertieyName, statement) \
  DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertieyName, statement)

#define DEFINE_STRING_SETTER(T_RIME_TYPE, name, assignment) \
  DEFINE_STRING_SETTER_IMPL_QJS(T_RIME_TYPE, name, assignment)

#define DEFINE_SETTER(T_RIME_TYPE, jsName, converter, assignment) \
  DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, converter, assignment)

#define DEFINE_CFUNCTION(funcName, funcBody) DEFINE_CFUNCTION_QJS(funcName, funcBody)

#define DEFINE_CFUNCTION_ARGC(funcName, expectingArgc, statements) \
  DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)

#define EXPORT_CLASS_IMPL(className, block1, block2, block3, block4) \
  EXPORT_CLASS_IMPL_QJS(className, EXPAND(block1), EXPAND(block2), EXPAND(block3), EXPAND(block4));

#define WITH_CONSTRUCTOR(funcName, argc) WITH_CONSTRUCTOR_QJS(funcName, argc)
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

// NOLINTBEGIN(cppcoreguidelines-macro-usage) function-like macro 'DEFINE_GETTER' used; consider a 'constexpr' template function
// =============== GETTER ===============
#define DEFINE_GETTER_IMPL_QJS(T_RIME_TYPE, propertieyName, statement)        \
  static JSValue get_##propertieyName(JSContext* ctx, JSValueConst thisVal) { \
    auto& engine = JsEngine<QjsValueRAII>::instance();                        \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                     \
      return engine.wrap(statement).release();                                \
    }                                                                         \
    return JS_UNDEFINED;                                                      \
  }

// =============== SETTER ===============
#define DEFINE_STRING_SETTER_IMPL_QJS(T_RIME_TYPE, name, assignment)             \
  static JSValue set_##name(JSContext* ctx, JSValueConst thisVal, JSValue val) { \
    auto& engine = JsEngine<QjsValueRAII>::instance();                           \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                        \
      auto str = engine.toStdString(QjsValueRAII(ctx, JS_DupValue(ctx, val)));   \
      if (!str.empty()) {                                                        \
        assignment;                                                              \
        return JS_UNDEFINED;                                                     \
      }                                                                          \
      auto* format = "%s.%s = rvalue: rvalue is not a string";                   \
      return JS_ThrowTypeError(ctx, format, #T_RIME_TYPE, #name);                \
    }                                                                            \
    auto* format = "Failed to unwrap the js object to a cpp %s object";          \
    return JS_ThrowTypeError(ctx, format, #T_RIME_TYPE);                         \
  }

#define DEFINE_SETTER_IMPL_QJS(T_RIME_TYPE, jsName, converter, assignment)         \
  static JSValue set_##jsName(JSContext* ctx, JSValueConst thisVal, JSValue val) { \
    auto& engine = JsEngine<QjsValueRAII>::instance();                             \
    if (auto obj = engine.unwrap<T_RIME_TYPE>(thisVal)) {                          \
      auto value = converter(QjsValueRAII(ctx, JS_DupValue(ctx, val)));            \
      assignment;                                                                  \
      return JS_UNDEFINED;                                                         \
    }                                                                              \
    auto* format = "Failed to unwrap the js object to a cpp %s object";            \
    return JS_ThrowTypeError(ctx, format, #T_RIME_TYPE);                           \
  }

// =============== FUNCTION ===============

#define DEFINE_CFUNCTION_QJS(funcName, funcBody)                                      \
  static JSValue funcName(JSContext* ctx, JSValue thisVal, int argc, JSValue* argv) { \
    auto& engine = JsEngine<QjsValueRAII>::instance();                                \
    try {                                                                             \
      funcBody;                                                                       \
    } catch (const JsException& e) {                                                  \
      return JS_ThrowTypeError(ctx, "%s", e.what());                                  \
    }                                                                                 \
  }

#define DEFINE_CFUNCTION_ARGC_QJS(funcName, expectingArgc, statements)                           \
  static JSValue funcName(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {  \
    if (argc < (expectingArgc)) {                                                                \
      return JS_ThrowSyntaxError(ctx, "%s(...) expects %d arguments", #funcName, expectingArgc); \
    }                                                                                            \
    auto& engine = JsEngine<QjsValueRAII>::instance();                                           \
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

#define WITH_CONSTRUCTOR_QJS(funcName, argc)            \
  inline static JSCFunction* constructorQjs = funcName; \
  inline static const int CONSTRUCTOR_ARGC = argc;
#define WITHOUT_CONSTRUCTOR_QJS                        \
  inline static JSCFunction* constructorQjs = nullptr; \
  inline static const int CONSTRUCTOR_ARGC = 0;

#define WITH_FINALIZER_QJS                                                             \
  inline static JSClassFinalizer* finalizerQjs = [](JSRuntime* rt, JSValueConst val) { \
    if (void* ptr = JS_GetOpaque(val, jsClassId)) {                                    \
      if (auto* ppObj = static_cast<std::shared_ptr<T_RIME_TYPE>*>(ptr)) {             \
        delete ppObj;                                                                  \
        JS_SetOpaque(val, nullptr);                                                    \
      }                                                                               \
    }                                                                                  \
  };
#define WITHOUT_FINALIZER_QJS inline static JSClassFinalizer* finalizerQjs = nullptr;

#define DEFINE_PROPERTY(name) JS_CGETSET_DEF(#name, get_##name, set_##name),

#define WITH_PROPERTIES_QJS(...)                                \
  inline static const JSCFunctionListEntry PROPERTIES_QJS[] = { \
      FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__)};                  \
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

#define DEFINE_FUNCTION_QJS(name, argc) JS_CFUNC_DEF(#name, static_cast<uint8_t>(argc), name),

#define WITH_FUNCTIONS_QJS(...)                                \
  inline static const JSCFunctionListEntry FUNCTIONS_QJS[] = { \
      FOR_EACH_PAIR(DEFINE_FUNCTION_QJS, __VA_ARGS__)};        \
  inline static const size_t FUNCTIONS_SIZE = sizeof(FUNCTIONS_QJS) / sizeof(FUNCTIONS_QJS[0]);
#define WITHOUT_FUNCTIONS_QJS                                    \
  inline static const JSCFunctionListEntry FUNCTIONS_QJS[] = {}; \
  inline static const size_t FUNCTIONS_SIZE = 0;
// NOLINTEND(cppcoreguidelines-macro-usage)
