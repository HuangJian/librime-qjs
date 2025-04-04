#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <memory>

#include "engines/engine_manager.h"
#include "environment.h"
#include "qjs_types.h"
#include "test_helper.hpp"
#include "test_switch.h"
#include "trie_data_helper.hpp"

using namespace rime;

template <typename T>
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

SETUP_JS_ENGINES(QuickJSTypesTest);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TYPED_TEST(QuickJSTypesTest, WrapUnwrapRimeTypes) {
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

  auto jsEngine = newOrShareEngine<TypeParam>();
  registerTypesToJsEngine(jsEngine);

  auto env = std::make_unique<Environment>(engine.get(), "namespace");
  TypeParam environment = jsEngine.wrap(env.get());

  auto folderPath = getFolderPath(__FILE__);
  auto jsEnvironment = jsEngine.toObject(environment);
  jsEngine.setObjectProperty(jsEnvironment, "currentFolder",
                             jsEngine.toJsString(folderPath.c_str()));

  auto candidate = New<SimpleCandidate>("mock", 0, 1, "text", "comment");
  jsEngine.setObjectProperty(jsEnvironment, "candidate",
                             jsEngine.template wrapShared<Candidate>(candidate));

  auto result = jsEngine.loadJsFile("types_test.js");
  auto global = jsEngine.getGlobalObject();
  auto jsFunc = jsEngine.getObjectProperty(jsEngine.toObject(global), "checkArgument");
  auto retValue =
      jsEngine.callFunction(jsEngine.toObject(jsFunc), jsEngine.toObject(global), 1, &environment);

  auto retJsEngine = jsEngine.getObjectProperty(jsEngine.toObject(retValue), "engine");
  auto* retEngine = jsEngine.template unwrap<Engine>(retJsEngine);
  ASSERT_EQ(retEngine, engine.get());
  ASSERT_EQ(retEngine->schema()->schema_name(), engine->schema()->schema_name());
  auto retJsCandidate = jsEngine.getObjectProperty(jsEngine.toObject(retValue), "candidate");
  an<Candidate> retCandidate = jsEngine.template unwrapShared<Candidate>(retJsCandidate);
  ASSERT_EQ(retCandidate->text(), "new text");
  ASSERT_EQ(retCandidate.get(), candidate.get());

  string greet;
  bool success = retEngine->schema()->config()->GetString("greet", &greet);
  ASSERT_TRUE(success);
  ASSERT_EQ(greet, "hello from js");

  Context* retContext = retEngine->context();
  ASSERT_EQ(retContext->input(), "world");

  // js code: env.newCandidate = new Candidate('js', 32, 100, 'the text', 'the comment', 888)
  auto retJsNewCandidate = jsEngine.getObjectProperty(jsEngine.toObject(retValue), "newCandidate");
  auto newCandidate = jsEngine.template unwrapShared<Candidate>(retJsNewCandidate);
  ASSERT_EQ(newCandidate->type(), "js");
  ASSERT_EQ(newCandidate->start(), 32);
  ASSERT_EQ(newCandidate->end(), 100);
  ASSERT_EQ(newCandidate->text(), "the text");
  ASSERT_EQ(newCandidate->comment(), "the comment");
  ASSERT_EQ(newCandidate->quality(), 888);

  // free all js objects
  TypeParam objects[] = {jsEnvironment, global,         jsFunc,           retValue,
                         retJsEngine,   retJsCandidate, retJsNewCandidate};
  for (auto obj : objects) {
    jsEngine.freeValue(obj);
  }
}
