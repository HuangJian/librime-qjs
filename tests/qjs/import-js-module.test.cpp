#include <gtest/gtest.h>
#include <quickjs.h>
#include <filesystem>

#include "engines/common.h"
#include "engines/quickjs/quickjs_code_loader.h"

class QuickJSModuleTest : public testing::Test {
protected:
  void SetUp() override {
    rt_ = JS_NewRuntime();
    ctx_ = JS_NewContext(rt_);

    JS_SetModuleLoaderFunc(rt_, nullptr, js_module_loader, nullptr);

    std::filesystem::path path(__FILE__);
    path = path.parent_path().parent_path() / "js";
    setQjsBaseFolder(path.generic_string().c_str());

    QuickJsEngineImpl::exposeLogToJsConsole(ctx_);
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

TEST_F(QuickJSModuleTest, ImportJsModuleFromAnotherJsFile) {
  auto* ctx = getContext();
  JSValue module = QuickJSCodeLoader::loadJsModuleToGlobalThis(ctx, "main.js");

  JSValue globalObj = JS_GetGlobalObject(ctx);
  JSValue myClass = JS_GetPropertyStr(ctx, globalObj, "MyClass");
  ASSERT_FALSE(JS_IsException(myClass));

  constexpr int A_NAMED_INT = 10;
  JSValue arg = JS_NewInt32(ctx, A_NAMED_INT);
  JSValue obj = JS_CallConstructor(ctx, myClass, 1, &arg);
  ASSERT_FALSE(JS_IsException(obj));

  JSValue greetArg = JS_NewString(ctx, "QuickJS");
  JSAtom greetAtom = JS_NewAtom(ctx, "greet");
  JSValue greetResult = JS_Invoke(ctx, obj, greetAtom, 1, &greetArg);
  ASSERT_FALSE(JS_IsException(greetResult));
  JS_FreeAtom(ctx, greetAtom);

  const char* str = JS_ToCString(ctx, greetResult);
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "Hello QuickJS!");
  JS_FreeCString(ctx, str);

  for (auto obj : {module, globalObj, myClass, arg, obj, greetArg, greetResult}) {
    JS_FreeValue(ctx, obj);
  }
}

TEST_F(QuickJSModuleTest, ImportJsModuleFromAnotherJsFileWithEngine) {
  auto& engine = JsEngine<QjsValueRAII>::instance();
  std::filesystem::path path = __FILE__;
  path = path.parent_path().parent_path() / "js";
  engine.setBaseFolderPath(path.generic_string().c_str());

  auto module = engine.loadJsFile("main.js");

  auto globalObj = engine.getGlobalObject();
  auto myClass = engine.getObjectProperty(globalObj, "MyClass");
  ASSERT_FALSE(engine.isException(myClass));

  constexpr int A_NAMED_INT = 10;
  auto arg = engine.wrap(A_NAMED_INT);
  QjsValueRAII args[] = {std::move(arg)};
  auto obj = engine.newClassInstance(myClass, 1, args);
  ASSERT_FALSE(engine.isException(obj));

  auto greetArg = engine.wrap("QuickJS");
  auto greeFunction = engine.getMethodOfClassOrInstance(myClass, obj, "greet");
  QjsValueRAII greetArgs[] = {std::move(greetArg)};
  auto greetResult = engine.callFunction(greeFunction, obj, 1, greetArgs);
  ASSERT_FALSE(engine.isException(greetResult));

  auto str = engine.toStdString(greetResult);
  ASSERT_FALSE(str.empty());
  EXPECT_STREQ(str.c_str(), "Hello QuickJS!");
}

TEST_F(QuickJSModuleTest, ImportJsModuleToNamespace) {
  auto* ctx = getContext();
  JSValue moduleNamespace = QuickJSCodeLoader::loadJsModuleToNamespace(ctx, "lib.js");
  ASSERT_FALSE(JS_IsException(moduleNamespace));

  // Get the greet function from the namespace
  JSValue greetFunc = JS_GetPropertyStr(ctx, moduleNamespace, "greet");
  ASSERT_FALSE(JS_IsException(greetFunc));

  JSValue arg = JS_NewString(ctx, "QuickJS");
  JSValue result = JS_Call(ctx, greetFunc, JS_UNDEFINED, 1, &arg);
  ASSERT_FALSE(JS_IsException(result));

  // Verify the result
  const char* str = JS_ToCString(ctx, result);
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "Hello QuickJS!");
  JS_FreeCString(ctx, str);

  for (auto obj : {moduleNamespace, greetFunc, arg, result}) {
    JS_FreeValue(ctx, obj);
  }
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSModuleTest, FindImportedClass) {
  auto* ctx = getContext();
  JSValue moduleNamespace =
      QuickJSCodeLoader::loadJsModuleToNamespace(ctx, "lib");  // loads ./dist/lib.esm.js
  ASSERT_FALSE(JS_IsException(moduleNamespace));

  JSValue myClass =
      QuickJSCodeLoader::getExportedClassByNameInModule(ctx, moduleNamespace, "MyClass");
  ASSERT_FALSE(JS_IsException(myClass));
  ASSERT_FALSE(JS_IsUndefined(myClass));

  JSValue classHavingMethodName =
      QuickJSCodeLoader::getExportedClassHavingMethodNameInModule(ctx, moduleNamespace, "myMethod");
  ASSERT_FALSE(JS_IsException(classHavingMethodName));
  // ASSERT_EQ(myClass.getPtr(), classHavingMethodName.getPtr()); // <-- this is not true, because the class is not the same object.

  for (auto clazz : {myClass, classHavingMethodName}) {
    JSValue proto = JS_GetPropertyStr(ctx, clazz, "prototype");
    ASSERT_FALSE(JS_IsException(proto));

    JSValue constructor = JS_GetPropertyStr(ctx, proto, "constructor");
    ASSERT_FALSE(JS_IsException(constructor));

    constexpr int A_NAMED_INT = 10;
    JSValue arg = JS_NewInt32(ctx, A_NAMED_INT);
    JSValue obj = JS_CallConstructor(ctx, constructor, 1, &arg);
    ASSERT_FALSE(JS_IsException(obj));

    JSValue myMethod = QuickJSCodeLoader::getMethodByNameInClass(ctx, clazz, "myMethod");
    ASSERT_FALSE(JS_IsException(myMethod));
    ASSERT_FALSE(JS_IsUndefined(myMethod));
    JSValue myMethodResult = JS_Call(ctx, myMethod, obj, 0, nullptr);

    int intResult = 0;
    JS_ToInt32(ctx, &intResult, myMethodResult);
    ASSERT_EQ(intResult, A_NAMED_INT + 1);

    for (auto obj : {proto, constructor, arg, obj, myMethod, myMethodResult}) {
      JS_FreeValue(ctx, obj);
    }
  }

  for (auto obj : {
           moduleNamespace,
           myClass,
           classHavingMethodName,
       }) {
    JS_FreeValue(ctx, obj);
  }
}

TEST_F(QuickJSModuleTest, ImportNodeModule) {
  auto* ctx = getContext();
  JSValue module = QuickJSCodeLoader::loadJsModuleToGlobalThis(ctx, "node-modules.test");
  ASSERT_FALSE(JS_IsException(module));
  JS_FreeValue(ctx, module);
}
