#include <JavaScriptCore/JavaScriptCore.h>
#include <gtest/gtest.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/gear/translator_commons.h>
#include <string>
#include <vector>

#include "../fake_translation.hpp"
#include "engines/common.h"
#include "qjs_translation.h"
#include "types/qjs_types.h"

class JscLoadBundledPluginTest : public testing::Test {
  void SetUp() override {
    auto& engine = JsEngine<JSValueRef>::instance();
    std::filesystem::path path(rime_get_api()->get_user_data_dir());
    path.append("js");
    engine.setBaseFolderPath(path.generic_string().c_str());

    registerTypesToJsEngine<JSValueRef>();
  }
};

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(JscLoadBundledPluginTest, RunEsmBundledTranslator) {
  auto& engine = JsEngine<JSValueRef>::instance();

  const auto* container = engine.loadJsFile("help_menu.esm");
  EXPECT_TRUE(engine.isObject(container)) << "JavaScript evaluation failed";

  const auto* clazz = engine.getJsClassHavingMethod(container, "translate");
  EXPECT_TRUE(clazz != nullptr);

  the<Engine> rimeEngine(Engine::Create());
  auto env = std::make_unique<Environment>(rimeEngine.get(), "help_menu");
  const auto* jsEnv = engine.wrap(env.get());
  JSObjectRef instance = engine.newClassInstance(engine.toObject(clazz), 1, &jsEnv);
  EXPECT_TRUE(instance != nullptr) << "Failed to create instance of class with translate method";

  // Verify the instance has the translate method
  JSValueRef translateMethod = engine.getObjectProperty(instance, "translate");
  EXPECT_TRUE(engine.isFunction(translateMethod));

  // Execute the translate method
  JSValueRef input = engine.wrap("/help");
  Segment segment;
  JSValueRef args[] = {input, engine.wrap(&segment), jsEnv};
  int size = sizeof(args) / sizeof(args[0]);
  JSValueRef translateResult = engine.callFunction(engine.toObject(translateMethod), instance, size,
                                                   static_cast<JSValueRef*>(args));
  EXPECT_TRUE(engine.isArray(translateResult));

  // Count items in translateResult
  size_t itemCount = engine.getArrayLength(translateResult);
  EXPECT_GT(itemCount, 9) << "translateResult should contain items";
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(JscLoadBundledPluginTest, RunEsmBundledFilter) {
  auto& engine = JsEngine<JSValueRef>::instance();

  // load the esm format bundled file
  const auto* container = engine.loadJsFile("sort_by_pinyin.esm");
  EXPECT_TRUE(engine.isObject(container)) << "JavaScript evaluation failed";

  const auto* clazz = engine.getJsClassHavingMethod(container, "filter");
  EXPECT_TRUE(clazz != nullptr);

  the<Engine> rimeEngine(Engine::Create());
  rimeEngine->context()->set_input("pinyin");
  auto env = std::make_unique<Environment>(rimeEngine.get(), "sort_by_pinyin");
  const auto* jsEnv = engine.wrap(env.get());
  JSObjectRef instance = engine.newClassInstance(engine.toObject(clazz), 1, &jsEnv);
  EXPECT_TRUE(instance != nullptr) << "Failed to create instance of class with filter method";

  // Verify the instance has the translate method
  JSValueRef filterMethod = engine.getObjectProperty(instance, "filter");
  EXPECT_TRUE(engine.isFunction(filterMethod));

  std::vector<an<Candidate>> candidates;
  candidates.push_back(New<SimpleCandidate>("mock", 0, 1, "text1", "[pinyin]"));
  candidates.push_back(New<SimpleCandidate>("mock", 0, 1, "text2", "〖pīn yīn〗"));
  candidates.push_back(New<SimpleCandidate>("mock", 0, 1, "text3", "［pin yin］"));
  candidates.push_back(New<SimpleCandidate>("user_phrase", 0, 1, "text4", ""));
  candidates.push_back(New<SimpleCandidate>("user_phrase", 0, 1, "text5"));

  const auto* jsCandidates = engine.newArray();
  for (size_t i = 0; i < candidates.size(); ++i) {
    JSValueRef jsCandidate = engine.wrap(candidates[i]);
    engine.insertItemToArray(jsCandidates, i, jsCandidate);
  }

  // Execute the translate method
  JSValueRef args[] = {jsCandidates, jsEnv};
  int size = sizeof(args) / sizeof(args[0]);
  JSValueRef filterResult = engine.callFunction(engine.toObject(filterMethod), instance, size,
                                                static_cast<JSValueRef*>(args));
  EXPECT_TRUE(engine.isArray(filterResult));
  size_t itemCount = engine.getArrayLength(filterResult);
  EXPECT_EQ(itemCount, candidates.size()) << "translateResult should contain items";
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(JscLoadBundledPluginTest, FilterTranslationWithJavaScriptCore) {
  auto& engine = JsEngine<JSValueRef>::instance();

  const auto* container = engine.loadJsFile("sort_by_pinyin");
  EXPECT_TRUE(engine.isObject(container)) << "JavaScript evaluation failed";

  const auto* clazz = engine.getJsClassHavingMethod(container, "filter");
  EXPECT_TRUE(clazz != nullptr);

  the<Engine> rimeEngine(Engine::Create());
  rimeEngine->context()->set_input("pinyin");
  auto env = std::make_unique<Environment>(rimeEngine.get(), "sort_by_pinyin");

  auto fakeTranslation = New<FakeTranslation>();
  fakeTranslation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "[pinyin]"));
  fakeTranslation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "〖pīn yīn〗"));
  fakeTranslation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "［pin yin］"));
  fakeTranslation->append(New<SimpleCandidate>("user_phrase", 0, 1, "text4", ""));
  fakeTranslation->append(New<SimpleCandidate>("user_phrase", 0, 1, "text5"));

  const auto* jsEnv = engine.wrap(env.get());
  JSObjectRef instance = engine.newClassInstance(engine.toObject(clazz), 1, &jsEnv);
  JSValueRef filterMethod = engine.getObjectProperty(instance, "filter");
  EXPECT_TRUE(engine.isFunction(filterMethod));

  QuickJSTranslation<JSValueRef> translation(fakeTranslation, instance, filterMethod, env.get());

  EXPECT_FALSE(translation.exhausted());
  EXPECT_TRUE(translation.Next());
  EXPECT_EQ(translation.Peek()->text(), "text2");
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(JscLoadBundledPluginTest, RunIifeBundledFilter) {
  auto& engine = JsEngine<JSValueRef>::instance();

  the<Engine> rimeEngine(Engine::Create());
  rimeEngine->context()->set_input("pinyin");
  auto env = std::make_unique<Environment>(rimeEngine.get(), "sort_by_pinyin");
  const auto* jsEnv = engine.wrap(env.get());
  std::vector<JSValueRef> args = {jsEnv};
  JSObjectRef instance = engine.createInstanceOfModule("sort_by_pinyin", args, "");
  EXPECT_TRUE(engine.isObject(instance));
  JSValueRef filterMethod = engine.getObjectProperty(instance, "filter");
  EXPECT_TRUE(engine.isFunction(filterMethod));

  auto fakeTranslation = New<FakeTranslation>();
  fakeTranslation->append(New<SimpleCandidate>("mock", 0, 1, "text1", "[pinyin]"));
  fakeTranslation->append(New<SimpleCandidate>("mock", 0, 1, "text2", "〖pīn yīn〗"));
  fakeTranslation->append(New<SimpleCandidate>("mock", 0, 1, "text3", "［pin yin］"));
  fakeTranslation->append(New<SimpleCandidate>("user_phrase", 0, 1, "text4", ""));
  fakeTranslation->append(New<SimpleCandidate>("user_phrase", 0, 1, "text5"));

  QuickJSTranslation<JSValueRef> translation(fakeTranslation, instance, filterMethod, env.get());

  EXPECT_FALSE(translation.exhausted());
  EXPECT_TRUE(translation.Next());
  EXPECT_EQ(translation.Peek()->text(), "text2");
}
