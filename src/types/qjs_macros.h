#ifndef RIME_QJS_MACROS_H_
#define RIME_QJS_MACROS_H_

#include <quickjs.h>
#include "jsstring_raii.h"

#define NO_PROPERTY_TO_DECLARE void useless() {};
#define NO_PROPERTY_TO_REGISTER {};
#define NO_FUNCTION_TO_REGISTER {};

// Helper macro for FOR_EACH implementation
#define EXPAND(...) __VA_ARGS__
#define FOR_EACH_1(macro, x) macro(x)
#define FOR_EACH_2(macro, x, ...) macro(x) EXPAND(FOR_EACH_1(macro, __VA_ARGS__))
#define FOR_EACH_3(macro, x, ...) macro(x) EXPAND(FOR_EACH_2(macro, __VA_ARGS__))
#define FOR_EACH_4(macro, x, ...) macro(x) EXPAND(FOR_EACH_3(macro, __VA_ARGS__))
#define FOR_EACH_5(macro, x, ...) macro(x) EXPAND(FOR_EACH_4(macro, __VA_ARGS__))
#define FOR_EACH_6(macro, x, ...) macro(x) EXPAND(FOR_EACH_5(macro, __VA_ARGS__))
#define FOR_EACH_7(macro, x, ...) macro(x) EXPAND(FOR_EACH_6(macro, __VA_ARGS__))
#define FOR_EACH_8(macro, x, ...) macro(x) EXPAND(FOR_EACH_7(macro, __VA_ARGS__))
#define FOR_EACH_9(macro, x, ...) macro(x) EXPAND(FOR_EACH_8(macro, __VA_ARGS__))
#define FOR_EACH_10(macro, x, ...) macro(x) EXPAND(FOR_EACH_9(macro, __VA_ARGS__))
#define FOR_EACH_11(macro, x, ...) macro(x) EXPAND(FOR_EACH_10(macro, __VA_ARGS__))
#define FOR_EACH_12(macro, x, ...) macro(x) EXPAND(FOR_EACH_11(macro, __VA_ARGS__))
#define FOR_EACH_13(macro, x, ...) macro(x) EXPAND(FOR_EACH_12(macro, __VA_ARGS__))
#define FOR_EACH_14(macro, x, ...) macro(x) EXPAND(FOR_EACH_13(macro, __VA_ARGS__))
#define FOR_EACH_15(macro, x, ...) macro(x) EXPAND(FOR_EACH_14(macro, __VA_ARGS__))
#define FOR_EACH_16(macro, x, ...) macro(x) EXPAND(FOR_EACH_15(macro, __VA_ARGS__))
#define FOR_EACH_17(macro, x, ...) macro(x) EXPAND(FOR_EACH_16(macro, __VA_ARGS__))
#define FOR_EACH_18(macro, x, ...) macro(x) EXPAND(FOR_EACH_17(macro, __VA_ARGS__))
#define FOR_EACH_19(macro, x, ...) macro(x) EXPAND(FOR_EACH_18(macro, __VA_ARGS__))
#define FOR_EACH_20(macro, x, ...) macro(x) EXPAND(FOR_EACH_19(macro, __VA_ARGS__))

// Get number of arguments
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,  N, ...) N
#define COUNT_ARGS(...) COUNT_ARGS_(__VA_ARGS__, \
  20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
  10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Select the appropriate FOR_EACH macro based on argument count
#define _FOR_EACH_N(N, macro, ...) FOR_EACH_##N(macro, __VA_ARGS__)
#define FOR_EACH_N(N, macro, ...) _FOR_EACH_N(N, macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_N(COUNT_ARGS(__VA_ARGS__), macro, __VA_ARGS__)

#define countof(x) (sizeof(x) / sizeof((x)[0]))

// ========== class declaration ==========
#define DECLARE_PROPERTIES(...) \
  FOR_EACH(DECLARE_GETTER_SETTER, __VA_ARGS__)

#define DECLARE_GETTER_SETTER(name) \
  static JSValue get_##name(JSContext* ctx, JSValueConst this_val); \
  static JSValue set_##name(JSContext* ctx, JSValueConst this_val, JSValue val);

#define DECLARE_GETTERS(...) \
  FOR_EACH(DECLARE_GETTER, __VA_ARGS__)

#define DECLARE_GETTER(name) \
  static JSValue get_##name(JSContext* ctx, JSValueConst this_val);

#define DECLARE_FUNCTION(name) \
  static JSValue name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

#define DECLARE_FUNCTIONS(...) \
  FOR_EACH(DECLARE_FUNCTION, __VA_ARGS__)

#define DECLARE_WRAP_UNWRAP_WITH_RAW_POINTER(class_name) \
  static JSValue Wrap(JSContext* ctx, class_name* obj); \
  static class_name* Unwrap(JSContext* ctx, JSValue value);

#define DECLARE_WRAP_UNWRAP_WITH_SHARED_POINTER(class_name) \
  static JSValue Wrap(JSContext* ctx, std::shared_ptr<class_name> obj); \
  static std::shared_ptr<class_name> Unwrap(JSContext* ctx, JSValue value);

#define DECLARE_JS_CLASS(class_name, declareProperties, declareFunctions, declareWrapUnwrap) \
namespace rime { \
class Qjs##class_name : public QjsTypeRegistry { \
public: \
  void Register(JSContext* ctx) override; \
  const char* GetClassName() const override { return #class_name; } \
\
  declareWrapUnwrap \
\
private: \
  declareProperties \
  declareFunctions \
}; \
} // namespace rime

#define DECLARE_JS_CLASS_WITH_RAW_POINTER(class_name, declareProperties, declareFunctions) \
  DECLARE_JS_CLASS(class_name, declareProperties, declareFunctions, DECLARE_WRAP_UNWRAP_WITH_RAW_POINTER(class_name))

#define DECLARE_JS_CLASS_WITH_SHARED_POINTER(class_name, declareProperties, declareFunctions) \
  DECLARE_JS_CLASS(class_name, declareProperties, declareFunctions, DECLARE_WRAP_UNWRAP_WITH_SHARED_POINTER(class_name))

// ========== class implementation ==========
#define DEFINE_PROPERTY(name) JS_CGETSET_DEF(#name, get_##name, set_##name),

#define DEFINE_PROPERTIES(...)                                             \
  const JSCFunctionListEntry properties[] = {                              \
    FOR_EACH(DEFINE_PROPERTY, __VA_ARGS__)                                 \
  };                                                                       \
  JS_SetPropertyFunctionList(ctx, proto, properties, countof(properties));

#define REGISTER_GETTER(name) JS_CGETSET_DEF(#name, get_##name, NULL),

#define DEFINE_GETTERS(...)                                                \
  const JSCFunctionListEntry properties[] = {                              \
    FOR_EACH(REGISTER_GETTER, __VA_ARGS__)                                 \
  };                                                                       \
  JS_SetPropertyFunctionList(ctx, proto, properties, countof(properties));

#define DEFINE_FUNCTIONS(...)                                              \
  const JSCFunctionListEntry jsMethods[] = {                               \
     __VA_ARGS__                                                           \
  };                                                                       \
  JS_SetPropertyFunctionList(ctx, proto, jsMethods, countof(jsMethods));

#define WRAP_UNWRAP_WITH_RAW_POINTER(class_name)                             \
[[nodiscard]]                                                                \
JSValue Qjs##class_name::Wrap(JSContext* ctx, class_name* obj) {             \
  if (!obj) return JS_NULL;                                                  \
  JSValue jsobj = JS_NewObjectClass(ctx, js_##class_name##_class_id);        \
  if (JS_IsException(jsobj)) {                                               \
    return jsobj;                                                            \
  }                                                                          \
  if (JS_SetOpaque(jsobj, obj) < 0) {                                        \
    JS_FreeValue(ctx, jsobj);                                                \
    return JS_ThrowInternalError(ctx,                                        \
      "Failed to set a raw pointer to a %s object", #class_name);            \
  };                                                                         \
  return jsobj;                                                              \
}                                                                            \
                                                                             \
[[nodiscard]]                                                                \
class_name* Qjs##class_name::Unwrap(JSContext* ctx, JSValue value) {         \
  if (auto ptr = JS_GetOpaque(value, js_##class_name##_class_id)) {          \
    return static_cast<class_name*>(ptr);                                    \
  }                                                                          \
  return nullptr;                                                            \
}                                                                            \
                                                                             \
static void js_##class_name##_finalizer(JSRuntime* rt, JSValue val) {        \
  DLOG(INFO) << "Calling js_" << #class_name "_finalizer."                   \
             << "The raw pointer is passed from the rime engine,"            \
             << "so do not free it here.";                                   \
}                                                                            \

#define WRAP_UNWRAP_WITH_SHARED_POINTER(class_name)                          \
[[nodiscard]]                                                                \
JSValue Qjs##class_name::Wrap(JSContext* ctx, std::shared_ptr<class_name> obj) { \
  if (!obj) return JS_NULL;                                                  \
  JSValue jsobj = JS_NewObjectClass(ctx, js_##class_name##_class_id);        \
  if (JS_IsException(jsobj)) {                                               \
    return jsobj;                                                            \
  }                                                                          \
  auto ptr = std::make_unique<std::shared_ptr<class_name>>(obj);             \
  if (JS_SetOpaque(jsobj, ptr.release()) < 0) {                              \
    JS_FreeValue(ctx, jsobj);                                                \
    return JS_ThrowInternalError(ctx,                                        \
      "Failed to set a shared pointer to a %s object", #class_name);         \
  };                                                                         \
  return jsobj;                                                              \
}                                                                            \
                                                                             \
[[nodiscard]]                                                                \
std::shared_ptr<class_name> Qjs##class_name::Unwrap(JSContext* ctx, JSValue value) { \
  if (auto ptr = JS_GetOpaque(value, js_##class_name##_class_id)) {          \
    if (auto sharedPtr = static_cast<std::shared_ptr<class_name>*>(ptr)) {   \
      return *sharedPtr;                                                     \
    }                                                                        \
  }                                                                          \
  return nullptr;                                                            \
}                                                                            \
static void js_##class_name##_finalizer(JSRuntime* rt, JSValue val) {        \
  DLOG(INFO) << "Calling js_" << #class_name << "_finalizer.";               \
  if (auto ptr = JS_GetOpaque(val, js_##class_name##_class_id)) {            \
    delete static_cast<std::shared_ptr<class_name>*>(ptr);                   \
  }                                                                          \
}                                                                            \

#define DEFINE_JS_CLASS(class_name, registerProperties, registerFunctions, defineWrapUnwrap)   \
static JSClassID js_##class_name##_class_id;                                 \
                                                                             \
defineWrapUnwrap;                                                            \
                                                                             \
static JSClassDef js_##class_name##_class = {                                \
  #class_name,                                                               \
  .finalizer = js_##class_name##_finalizer                                   \
};                                                                           \
                                                                             \
void Qjs##class_name::Register(JSContext* ctx) {                             \
  auto rt = QjsHelper::getInstance().getRuntime();                           \
  JS_NewClassID(rt, &js_##class_name##_class_id);                            \
  JS_NewClass(rt, js_##class_name##_class_id, &js_##class_name##_class);     \
                                                                             \
  JSValue proto = JS_NewObject(ctx);                                         \
                                                                             \
  registerProperties;                                                        \
  registerFunctions;                                                         \
                                                                             \
  JS_SetClassProto(ctx, js_##class_name##_class_id, proto);                  \
}

#define DEFINE_JS_CLASS_WITH_RAW_POINTER(class_name, registerProperties, registerFunctions)   \
  DEFINE_JS_CLASS(class_name, EXPAND(registerProperties), EXPAND(registerFunctions), WRAP_UNWRAP_WITH_RAW_POINTER(class_name))

#define DEFINE_JS_CLASS_WITH_SHARED_POINTER(class_name, registerProperties, registerFunctions) \
  DEFINE_JS_CLASS(class_name, EXPAND(registerProperties), EXPAND(registerFunctions), WRAP_UNWRAP_WITH_SHARED_POINTER(class_name))

// to define a js getter with the same name of the c++ getter
#define DEFINE_GETTER(class_name, name, type, converter)                \
  [[nodiscard]]                                                         \
  JSValue Qjs##class_name::get_##name(JSContext* ctx, JSValueConst this_val) { \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      return converter(ctx, obj->name());                              \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

// to define a js getter with a different name of the c++ getter
#define DEFINE_GETTER_2(class_name, jsName, cppName, type, converter)   \
  [[nodiscard]]                                                         \
  JSValue Qjs##class_name::get_##jsName(JSContext* ctx, JSValueConst this_val) { \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      return converter(ctx, obj->cppName());                              \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

#define DEFINE_STRING_SETTER(class_name, name, assignment)                          \
  JSValue Qjs##class_name::set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      if (const char* str = JS_ToCString(ctx, val)) {                  \
        assignment                                                      \
        JS_FreeCString(ctx, str);                                      \
      }                                                                \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

#define DEFINE_NUMERIC_SETTER(class_name, name, type, converter)                    \
  JSValue Qjs##class_name::set_##name(JSContext* ctx, JSValueConst this_val, JSValue val) { \
    if (auto obj = Unwrap(ctx, this_val)) {                            \
      type value;                                                      \
      if (converter(ctx, &value, val) == 0) {                         \
        obj->set_##name(value);                                        \
      }                                                                \
    }                                                                  \
    return JS_UNDEFINED;                                               \
  }

#define DEF_FUNC(class_name, func_name, statements)                            \
[[nodiscard]]                                                                  \
JSValue Qjs##class_name::func_name(                                            \
  JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {       \
                                                                               \
  auto obj = Unwrap(ctx, this_val);                                            \
  if (!obj) return JS_UNDEFINED;                                               \
                                                                               \
  statements;                                                                  \
}

// to define a js function with only one parameter of the string type
#define DEF_FUNC_WITH_SINGLE_STRING_PARAM(class_name, func_name, statements)   \
[[nodiscard]]                                                                  \
JSValue Qjs##class_name::func_name(                                            \
  JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {       \
                                                                               \
  if (argc < 1) return JS_UNDEFINED;                                           \
                                                                               \
  auto obj = Unwrap(ctx, this_val);                                            \
  if (!obj) return JS_UNDEFINED;                                               \
                                                                               \
  JSStringRAII param(JS_ToCString(ctx, argv[0]));                              \
  statements;                                                                  \
}

// to define a js function with 2 parameters:
// the first one must be of string type,
// but the second one's type is not limited
#define DEF_FUNC_WITH_STRING_AND_ANOTHER(class_name, func_name, statements)    \
[[nodiscard]]                                                                  \
JSValue Qjs##class_name::func_name(                                            \
  JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {       \
                                                                               \
  if (argc < 2) return JS_FALSE;                                               \
                                                                               \
  auto obj = Unwrap(ctx, this_val);                                            \
  if (!obj) return JS_FALSE;                                                   \
                                                                               \
  JSStringRAII firstParam(JS_ToCString(ctx, argv[0]));                         \
  statements;                                                                  \
}

#endif  // RIME_QJS_MACROS_H_
