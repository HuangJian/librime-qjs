#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>

#include <quickjs.h>
#include "engines/engine_manager.h"
#include "environment.h"
#include "test_helper.hpp"
#include "trie_data_helper.hpp"

using namespace rime;

class QuickJSTypesTest : public ::testing::Test {
private:
  TrieDataHelper trieDataHelper_ =
      TrieDataHelper(getFolderPath(__FILE__).c_str(), "dummy_dict.txt");

protected:
  void SetUp() override { trieDataHelper_.createDummyTextFile(); }

  void TearDown() override {
    trieDataHelper_.cleanupDummyFiles();
    std::remove("tests/dumm.bin");  // the file generated in js
  }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTypesTest, WrapUnwrapRimeTypes) {
  the<Engine> engine(Engine::Create());
  ASSERT_TRUE(engine->schema() != nullptr);
  auto* config = engine->schema()->config();
  ASSERT_TRUE(config != nullptr);
  config->SetBool("key1", true);
  config->SetBool("key2", false);
  constexpr int A_INT_NUMBER = 666;
  config->SetInt("key3", A_INT_NUMBER);
  constexpr double A_DOUBLE_NUMBER = 0.999;
  config->SetDouble("key4", A_DOUBLE_NUMBER);
  config->SetString("key5", "string");

  auto list = New<ConfigList>();
  list->Append(New<ConfigValue>("item1"));
  list->Append(New<ConfigValue>("item2"));
  list->Append(New<ConfigValue>("item3"));
  config->SetItem("list", list);

  auto* context = engine->context();
  ASSERT_TRUE(context != nullptr);
  context->set_input("hello");

  auto& jsEngine = getJsEngine<JSValue>();
  JSContext* ctx = jsEngine.getContext();

  auto env = std::make_shared<Environment>(engine.get(), "namespace");
  JSValue environment = jsEngine.wrapShared(env);

  auto folderPath = getFolderPath(__FILE__);
  JS_SetPropertyStr(ctx, environment, "currentFolder", JS_NewString(ctx, folderPath.c_str()));

  auto candidate = New<SimpleCandidate>("mock", 0, 1, "text", "comment");
  JS_SetPropertyStr(ctx, environment, "candidate", jsEngine.wrapShared<Candidate>(candidate));

  JSValue module = QuickJSCodeLoader::loadJsModuleToGlobalThis(ctx, "types_test.js");

  JSValue global = JS_GetGlobalObject(ctx);
  JSValue jsFunc = JS_GetPropertyStr(ctx, global, "checkArgument");
  JSValue retValue = JS_Call(ctx, jsFunc, JS_UNDEFINED, 1, &environment);

  JSValue retJsEngine = JS_GetPropertyStr(ctx, retValue, "engine");
  auto* retEngine = jsEngine.unwrap<Engine>(retJsEngine);
  ASSERT_EQ(retEngine, engine.get());
  ASSERT_EQ(retEngine->schema()->schema_name(), engine->schema()->schema_name());
  JSValue retJsCandidate = JS_GetPropertyStr(ctx, retValue, "candidate");
  an<Candidate> retCandidate = jsEngine.unwrapShared<Candidate>(retJsCandidate);
  ASSERT_EQ(retCandidate->text(), "new text");
  ASSERT_EQ(retCandidate.get(), candidate.get());

  string greet;
  bool success = retEngine->schema()->config()->GetString("greet", &greet);
  ASSERT_TRUE(success);
  ASSERT_EQ(greet, "hello from js");

  Context* retContext = retEngine->context();
  ASSERT_EQ(retContext->input(), "world");

  // js code: env.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)
  JSValue retJsNewCandidate = JS_GetPropertyStr(ctx, retValue, "newCandidate");
  auto newCandidate = jsEngine.unwrapShared<Candidate>(retJsNewCandidate);
  ASSERT_EQ(newCandidate->type(), "js");
  ASSERT_EQ(newCandidate->start(), 32);
  ASSERT_EQ(newCandidate->end(), 100);
  ASSERT_EQ(newCandidate->text(), "the text");
  ASSERT_EQ(newCandidate->comment(), "the comment");
  ASSERT_EQ(newCandidate->quality(), 888);

  // free all js objects
  JSValue objects[] = {environment, module,      global,         jsFunc,
                       retValue,    retJsEngine, retJsCandidate, retJsNewCandidate};
  for (auto obj : objects) {
    jsEngine.freeValue(obj);
  }
}
