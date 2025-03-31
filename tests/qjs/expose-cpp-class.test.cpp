#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <utility>
#include "engines/js_macros.h"
#include "engines/quickjs/quickjs_engine.h"

#include <quickjs.h>

class MyClass {
public:
  MyClass(std::string name) : name_(std::move(name)) {}

  [[nodiscard]] std::string sayHello() const {
    std::stringstream ss;
    ss << "Hello, " << name_ << "!";
    return ss.str();
  }

  [[nodiscard]] std::string getName() const { return name_; }

  void setName(const std::string& name) { name_ = name; }

private:
  std::string name_;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static JSClassID jsMyclassClassId;

// C++ Class Instance Finalizer (called when the object is GC'd)
static void jsMyclassFinalizer(JSRuntime* rt, JSValue val) {
  auto* instance = static_cast<MyClass*>(JS_GetOpaque(val, jsMyclassClassId));
  delete instance;
}

// Constructor for MyClass
static JSValue jsMyclassConstructor(JSContext* ctx,
                                    JSValueConst newTarget,
                                    int argc,
                                    JSValueConst* argv) {
  const char* name = nullptr;

  if (argc > 0 && JS_IsString(argv[0])) {
    name = JS_ToCString(ctx, argv[0]);
  }

  if (name == nullptr) {
    return JS_ThrowTypeError(ctx, "Expected a string as the first argument");
  }

  auto* instance = new MyClass(name);
  JSValue obj =
      JS_NewObjectClass(ctx, static_cast<int>(jsMyclassClassId));  // Use the class ID here
  if (JS_IsException(obj)) {
    delete instance;
    return obj;
  }

  JS_SetOpaque(obj, instance);
  JS_FreeCString(ctx, name);
  return obj;
}

// Method: sayHello
static JSValue jsMyclassSayHello(JSContext* ctx,
                                 JSValueConst thisVal,
                                 int argc,
                                 JSValueConst* argv) {
  auto* instance = static_cast<MyClass*>(JS_GetOpaque(thisVal, jsMyclassClassId));
  if (instance == nullptr) {
    return JS_ThrowTypeError(ctx, "Invalid MyClass instance");
  }

  return JS_NewString(ctx, instance->sayHello().c_str());
}

// Method: getName
static JSValue jsMyclassGetName(JSContext* ctx,
                                JSValueConst thisVal,
                                int argc,
                                JSValueConst* argv) {
  auto* instance = static_cast<MyClass*>(JS_GetOpaque(thisVal, jsMyclassClassId));
  if (instance == nullptr) {
    return JS_ThrowTypeError(ctx, "Invalid MyClass instance");
  }

  return JS_NewString(ctx, instance->getName().c_str());
}

// Method: setName
static JSValue jsMyclassSetName(JSContext* ctx,
                                JSValueConst thisVal,
                                int argc,
                                JSValueConst* argv) {
  auto* instance = static_cast<MyClass*>(JS_GetOpaque(thisVal, jsMyclassClassId));
  if (instance == nullptr) {
    return JS_ThrowTypeError(ctx, "Invalid MyClass instance");
  }

  const char* name = JS_ToCString(ctx, argv[0]);
  if (name == nullptr) {
    return JS_ThrowTypeError(ctx, "Expected a string");
  }

  instance->setName(name);
  JS_FreeCString(ctx, name);
  return JS_UNDEFINED;
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static JSClassDef jsMyclassClass = {
    .class_name = "MyClass",
    .finalizer = jsMyclassFinalizer,
    .gc_mark = nullptr,
    .call = nullptr,
    .exotic = nullptr,
};

// Export the class and methods
static const JSCFunctionListEntry JS_MYCLASS_PROTO_FUNCS[] = {
    JS_CFUNC_DEF("sayHello", 0, jsMyclassSayHello),
    JS_CFUNC_DEF("getName", 0, jsMyclassGetName),
    JS_CFUNC_DEF("setName", 1, jsMyclassSetName),
};

void registerMyclass(JSContext* ctx) {
  auto* rt = JS_GetRuntime(ctx);
  JS_NewClassID(rt, &jsMyclassClassId);
  JS_NewClass(rt, jsMyclassClassId, &jsMyclassClass);

  JSValue proto = JS_NewObject(ctx);
  JS_DupValue(ctx, proto);  // Duplicate the reference for safety

  JS_SetPropertyFunctionList(ctx, proto,
                             static_cast<const JSCFunctionListEntry*>(JS_MYCLASS_PROTO_FUNCS),
                             countof(JS_MYCLASS_PROTO_FUNCS));

  JSValue constructor =
      JS_NewCFunction2(ctx, jsMyclassConstructor, "MyClass", 1, JS_CFUNC_constructor, 0);
  JS_DupValue(ctx, constructor);  // Duplicate the reference for safety
  JS_SetConstructor(ctx, constructor, proto);
  JS_SetClassProto(ctx, jsMyclassClassId, proto);

  // Expose to the global object
  auto globalObj = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, globalObj, "MyClass", constructor);

  JS_FreeValue(ctx, globalObj);
  JS_FreeValue(ctx, proto);
  JS_FreeValue(ctx, constructor);
}

class QuickJSExposeClassTest : public ::testing::Test {
protected:
  void SetUp() override {
    auto& jsEngine = JsEngine<JSValue>::getInstance();
    auto& ctx = jsEngine.getContext();
    registerMyclass(ctx);
  }
};

TEST_F(QuickJSExposeClassTest, TestExposeClassToQuickJS) {
  const char* script = R"(
        function testExposedCppClass() {
            let ret;
            const obj = new MyClass("QuickJS");
            ret = obj.sayHello();
            obj.setName("Trae");
            ret += ' ' + obj.sayHello();
            return ret;
        }
        testExposedCppClass();  // Execute immediately to avoid reference issues
    )";

  auto& jsEngine = JsEngine<JSValue>::getInstance();
  auto& ctx = jsEngine.getContext();
  JSValue result = JS_Eval(ctx, script, strlen(script), "<input>", JS_EVAL_TYPE_GLOBAL);
  // Handle the result
  const char* resultStr = JS_ToCString(ctx, result);
  EXPECT_STREQ(resultStr, "Hello, QuickJS! Hello, Trae!");
  JS_FreeCString(ctx, resultStr);
  JS_FreeValue(ctx, result);
}
