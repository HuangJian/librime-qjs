#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/translation.h>
#include <vector>

#include "fake_translation.hpp"
#include "jsvalue_raii.hpp"
#include "qjs_helper.h"
#include "qjs_translation.h"
#include "quickjs.h"

using namespace rime;

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

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslationTest, Initialize) {
  auto translation = createMockTranslation();
  auto qjsTranslation =
      New<QuickJSTranslation>(translation, JS_UNDEFINED, JS_UNDEFINED, JS_UNDEFINED);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslationTest, FilterCandidates) {
  auto translation = createMockTranslation();
  const char* jsCode = R"(
        function filterCandidates(candidates, env) {
            console.log(`filterCandidates: ${candidates.length}`)
            console.log(`env.expectingText: ${env.expectingText}`)
            return candidates.filter((it, idx) => it.text === env.expectingText)
        }
    )";

  auto* ctx = QjsHelper::getInstance().getContext();
  JSValueRAII result(JS_Eval(ctx, jsCode, strlen(jsCode), "<input>", JS_EVAL_TYPE_GLOBAL));
  JSValueRAII global(JS_GetGlobalObject(ctx));
  JSValueRAII filterFunc(JS_GetPropertyStr(ctx, global, "filterCandidates"));

  JSValueRAII env(JS_NewObject(ctx));
  JS_SetPropertyStr(ctx, env, "expectingText", JS_NewString(ctx, "text2"));

  auto qjsTranslation = New<QuickJSTranslation>(translation, JS_UNDEFINED, filterFunc, env);
  auto candidate = qjsTranslation->Peek();

  ASSERT_TRUE(candidate != nullptr);
  EXPECT_EQ(candidate->text(), "text2");
  ASSERT_TRUE(qjsTranslation->Next());
  EXPECT_TRUE(qjsTranslation->exhausted());
  candidate = qjsTranslation->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(qjsTranslation->Next());
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslationTest, EmptyTranslation) {
  auto translation = New<FakeTranslation>();
  auto qjsTranslation =
      New<QuickJSTranslation>(translation, JS_UNDEFINED, JS_UNDEFINED, JS_UNDEFINED);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(QuickJSTranslationTest, CandidatesShouldBeFreed) {
  std::vector<an<Candidate>> candidates;
  {
    auto translation = createMockTranslation();
    const char* jsCode = R"(
          function filterCandidates(candidates, env) {
            candidates.push(new Candidate('test', 0, 1, 'new Candidate from js', 'the comment'))
            return candidates
          }
      )";

    auto* ctx = QjsHelper::getInstance().getContext();
    JSValueRAII result(JS_Eval(ctx, jsCode, strlen(jsCode), "<input>", JS_EVAL_TYPE_GLOBAL));
    JSValueRAII global(JS_GetGlobalObject(ctx));
    JSValueRAII filterFunc(JS_GetPropertyStr(ctx, global, "filterCandidates"));

    JSValueRAII env(JS_NewObject(ctx));

    auto qjsTranslation = New<QuickJSTranslation>(translation, JS_UNDEFINED, filterFunc, env);

    while (auto candidate = qjsTranslation->exhausted() ? nullptr : qjsTranslation->Peek()) {
      candidates.push_back(candidate);
      qjsTranslation->Next();
    }

    candidates.push_back(New<SimpleCandidate>("mock", 0, 1, "text4", "comment4"));
  }

  ASSERT_EQ(candidates.size(), 5);

  for (auto& candidate : candidates) {
    ASSERT_EQ(candidate.use_count(), 1);
  }
}

TEST_F(QuickJSTranslationTest, NoReturnValueShouldNotCrash) {
  auto translation = createMockTranslation();
  const char* jsCode = "( function noReturn() { } )";

  auto* ctx = QjsHelper::getInstance().getContext();
  JSValueRAII result = JS_Eval(ctx, jsCode, strlen(jsCode), "<input>", JS_EVAL_TYPE_GLOBAL);
  JSValueRAII global = JS_GetGlobalObject(ctx);
  JSValueRAII filterFunc = JS_GetPropertyStr(ctx, global, "noReturn");

  auto qjsTranslation =
      New<QuickJSTranslation>(translation, JS_UNDEFINED, filterFunc, JS_UNDEFINED);
  EXPECT_TRUE(qjsTranslation->exhausted());
  EXPECT_FALSE(qjsTranslation->Next());
  EXPECT_EQ(qjsTranslation->Peek(), nullptr);
}
