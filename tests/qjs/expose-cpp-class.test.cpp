#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <quickjs.h>

#include "engines/common.h"

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
  JS_SetPropertyFunctionList(ctx, proto,
                             static_cast<const JSCFunctionListEntry*>(JS_MYCLASS_PROTO_FUNCS),
                             countof(JS_MYCLASS_PROTO_FUNCS));

  JSValue constructor =
      JS_NewCFunction2(ctx, jsMyclassConstructor, "MyClass", 1, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, constructor, proto);
  JS_SetClassProto(ctx, jsMyclassClassId, proto);

  // Expose to the global object
  auto globalObj = JS_GetGlobalObject(ctx);
  JS_SetPropertyStr(ctx, globalObj, "MyClass", constructor);

  JS_FreeValue(ctx, globalObj);
}

class QuickJSExposeClassTest : public ::testing::Test {
protected:
  void SetUp() override {
    rt_ = JS_NewRuntime();
    ctx_ = JS_NewContext(rt_);

    registerMyclass(ctx_);
  }
  void TearDown() override {
    JS_FreeContext(ctx_);
    JS_FreeRuntime(rt_);
  }
  JSContext* getContext() { return ctx_; }

private:
  JSRuntime* rt_{nullptr};
  JSContext* ctx_{nullptr};
};

constexpr const char* SCRIPT = R"(
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

TEST_F(QuickJSExposeClassTest, TestExposeClassToQuickJS) {
  auto* ctx = getContext();
  JSValue result = JS_Eval(ctx, SCRIPT, strlen(SCRIPT), "<input>", JS_EVAL_TYPE_GLOBAL);
  // Handle the result
  const char* resultStr = JS_ToCString(ctx, result);
  EXPECT_STREQ(resultStr, "Hello, QuickJS! Hello, Trae!");
  JS_FreeCString(ctx, resultStr);
  JS_FreeValue(ctx, result);
}

template <>
class JsWrapper<MyClass> {
  JS_API_DEFINE_CFUNCTION_ARGC(sayHello, 0, {
    auto obj = engine.unwrap<MyClass>(thisVal);
    return engine.wrap(obj->sayHello());
  });
  JS_API_DEFINE_CFUNCTION_ARGC(getName, 0, {
    auto obj = engine.unwrap<MyClass>(thisVal);
    return engine.wrap(obj->getName());
  });
  JS_API_DEFINE_CFUNCTION_ARGC(setName, 1, {
    auto obj = engine.unwrap<MyClass>(thisVal);
    obj->setName(engine.toStdString(argv[0]));
    return engine.undefined();
  });
  JS_API_DEFINE_CFUNCTION_ARGC(makeMyClass, 1, {
    auto name = engine.toStdString(argv[0]);
    return engine.wrap(std::make_shared<MyClass>(name));
  });

public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(MyClass,
                                          JS_API_WITH_CONSTRUCTOR(makeMyClass),
                                          JS_API_WITH_PROPERTIES(),
                                          JS_API_WITH_GETTERS(),
                                          JS_API_WITH_FUNCTIONS(sayHello, getName, setName));
};

TEST_F(QuickJSExposeClassTest, TestExposeClassToQuickJSWithEngine) {
  auto& engine = JsEngine<JSValue>::instance();
  engine.registerType<MyClass>();

  auto result = engine.eval(SCRIPT);
  auto str = engine.toStdString(result);
  EXPECT_STREQ(str.c_str(), "Hello, QuickJS! Hello, Trae!");
  engine.freeValue(result);
}
