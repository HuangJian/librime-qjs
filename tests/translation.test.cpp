#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/engine.h>
#include <rime/translation.h>

#include "environment.h"
#include "fake_translation.hpp"
#include "gears/qjs_translation.h"
#include "test_switch.h"

using namespace rime;

template <typename T>
class QuickJSTranslationTest : public ::testing::Test {
protected:
  static an<Translation> createMockTranslation() {
    auto translation = New<FakeTranslation>();
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));
    return translation;
  }
};

SETUP_JS_ENGINES(QuickJSTranslationTest);

TYPED_TEST(QuickJSTranslationTest, Initialize) {
  auto& jsEngine = JsEngine<TypeParam>::instance();
  auto translation = this->createMockTranslation();
  the<Engine> engine(Engine::Create());
  Environment env(*engine, "test");
  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), TypeParam(), env);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TYPED_TEST(QuickJSTranslationTest, FilterCandidates) {
  auto translation = this->createMockTranslation();
  const char* jsCode = R"(
        function filterCandidates(candidates, env) {
            console.log(`filterCandidates: ${candidates.length}`)
            console.log(`env.namespace: ${env.namespace}`)
            const ret = candidates.filter((it, idx) => {
              console.log(`it.text = ${it.text}`)
              return it.text === 'text2'
            })
            console.log(`ret.length: ${ret.length}`)
            return ret
        }
    )";

  auto& jsEngine = JsEngine<TypeParam>::instance();
  auto result = jsEngine.eval(jsCode, "<input>");
  auto global = jsEngine.getGlobalObject();
  auto filterFunc = jsEngine.getObjectProperty(jsEngine.toObject(global), "filterCandidates");

  the<Engine> engine(Engine::Create());
  Environment env(*engine, "test");
  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), filterFunc, env);
  auto candidate = qjsTranslation->Peek();

  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "text2");
  ASSERT_TRUE(qjsTranslation->Next());
  EXPECT_TRUE(qjsTranslation->exhausted());
  candidate = qjsTranslation->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(qjsTranslation->Next());

  jsEngine.freeValue(global);
  jsEngine.freeValue(result);
  jsEngine.freeValue(filterFunc);
}

TYPED_TEST(QuickJSTranslationTest, EmptyTranslation) {
  auto& jsEngine = JsEngine<TypeParam>::instance();
  auto translation = New<FakeTranslation>();
  the<Engine> engine(Engine::Create());
  Environment env(*engine, "test");
  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), TypeParam(), env);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

TYPED_TEST(QuickJSTranslationTest, NoReturnValueShouldNotCrash) {
  auto& jsEngine = JsEngine<TypeParam>::instance();
  auto translation = this->createMockTranslation();

  const char* jsCode = "function noReturn() { }";
  auto result = jsEngine.eval(jsCode, "<input>");
  auto global = jsEngine.getGlobalObject();
  auto filterFunc = jsEngine.getObjectProperty(jsEngine.toObject(global), "noReturn");

  the<Engine> engine(Engine::Create());
  Environment env(*engine, "test");
  auto qjsTranslation =
      New<QuickJSTranslation<TypeParam>>(translation, TypeParam(), filterFunc, env);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);

  jsEngine.freeValue(filterFunc, result, global);
}
