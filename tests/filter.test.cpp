#include <gtest/gtest.h>
#include <rime/candidate.h>
#include <rime/config/config_component.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/schema.h>
#include <rime/translation.h>

#include "fake_translation.hpp"
#include "gears/qjs_filter.h"
#include "test_switch.h"

using namespace rime;

template <typename T>
class QuickJSFilterTest : public ::testing::Test {
public:
  static an<Translation> createMockTranslation() {
    auto translation = New<FakeTranslation>();
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "comment1"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "comment2"));
    translation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "comment3"));
    return translation;
  }
};

SETUP_JS_ENGINES(QuickJSFilterTest);

template <typename T>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static std::shared_ptr<rime::Translation> doFilterInJs(const std::string& nameSpace) {
  the<Engine> engine(Engine::Create());

  auto* config = engine->schema()->config();
  config->SetString("greet", "hello from c++");
  config->SetString("expectingText", "text2");

  Ticket ticket(engine.get(), "filter", std::string("qjs_filter@") + nameSpace);
  Environment env(*engine, nameSpace);
  auto filter = New<QuickJSFilter<T>>(ticket, env);
  auto translation = QuickJSFilterTest<T>::createMockTranslation();
  return filter->apply(translation, env);
}

static void checkFilteredValues(const std::shared_ptr<rime::Translation>& filtered) {
  ASSERT_TRUE(filtered != nullptr);

  auto candidate = filtered->Peek();
  ASSERT_TRUE(candidate != nullptr);
  ASSERT_EQ(candidate->text(), "text2");

  filtered->Next();
  EXPECT_TRUE(filtered->exhausted());
  candidate = filtered->Peek();
  ASSERT_TRUE(candidate == nullptr);
  ASSERT_FALSE(filtered->Next());
}

TYPED_TEST(QuickJSFilterTest, ApplyFilter) {
  auto filtered = doFilterInJs<TypeParam>("filter_test");
  checkFilteredValues(filtered);
}

TYPED_TEST(QuickJSFilterTest, TestRestartEngine) {
  JsEngine<TypeParam>::shutdown();

  std::filesystem::path path(rime_get_api()->get_user_data_dir());
  path.append("js");

  auto& engine = JsEngine<TypeParam>::instance();
  registerTypesToJsEngine<TypeParam>();
  engine.setBaseFolderPath(path.generic_string().c_str());

  auto filtered = doFilterInJs<TypeParam>("filter_test");
  checkFilteredValues(filtered);
}

TYPED_TEST(QuickJSFilterTest, CheckAppblicable) {
  auto source = QuickJSFilterTest<TypeParam>::createMockTranslation();
  auto filtered = doFilterInJs<TypeParam>("filter_is_applicable");

  while (!source->exhausted()) {
    ASSERT_FALSE(filtered->exhausted());
    ASSERT_STREQ(filtered->Peek()->text().c_str(), source->Peek()->text().c_str());
    source->Next();
    filtered->Next();
  }
  ASSERT_TRUE(filtered->exhausted());
}

TYPED_TEST(QuickJSFilterTest, TestFastFilter) {
  auto filtered = doFilterInJs<TypeParam>("fast_filter");

  ASSERT_FALSE(filtered->exhausted());
  ASSERT_STREQ(filtered->Peek()->text().c_str(), "text1");
  ASSERT_TRUE(filtered->Next());
  ASSERT_STREQ(filtered->Peek()->text().c_str(), "text3");
  ASSERT_FALSE(filtered->Next());
  ASSERT_TRUE(filtered->exhausted());
  ASSERT_TRUE(filtered->Peek() == nullptr);
}

TYPED_TEST(QuickJSFilterTest, TestFastFilterReturnIterator) {
  auto filtered = doFilterInJs<TypeParam>("fast_filter.iterator");
  auto expected = QuickJSFilterTest<TypeParam>::createMockTranslation();

  while (!expected->exhausted()) {
    ASSERT_FALSE(filtered->exhausted());
    ASSERT_STREQ(filtered->Peek()->text().c_str(), expected->Peek()->text().c_str());
    expected->Next();
    filtered->Next();
  }
  ASSERT_TRUE(filtered->exhausted());
}
