#include <gtest/gtest.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include <rime/ticket.h>

#include "environment.h"
#include "gears/qjs_filter.h"
#include "qjs_component.h"
#include "test_switch.h"

using namespace rime;

template <typename T>
class MockFilter {
public:
  MockFilter(const MockFilter&) = delete;
  MockFilter(MockFilter&&) = delete;
  MockFilter& operator=(const MockFilter&) = delete;
  MockFilter& operator=(MockFilter&&) = delete;

  explicit MockFilter(const Ticket& ticket, const Environment& environment) {
    LOG(INFO) << "MockFilter created with ticket: " << ticket.name_space;
  };
  ~MockFilter() { LOG(INFO) << "MockFilter destroyed"; }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  an<Translation> apply(an<Translation> translation, const Environment& environment) {
    return translation;
  }
};

template <typename T>
class QuickJSComponentTest : public ::testing::Test {};

SETUP_JS_ENGINES(QuickJSComponentTest);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TYPED_TEST(QuickJSComponentTest, ShareComponentAcrossRimeSessions) {
  QuickJSComponent<MockFilter<TypeParam>, Filter, TypeParam> component;

  the<Engine> engine1(Engine::Create());
  Ticket ticket(engine1.get(), "test_namespace", "test");

  auto* instance1 = component.Create(ticket);
  auto actualInstance1 = instance1->actual();
  delete instance1;  // Rime session 1 ends

  auto* instance2 = component.Create(ticket);
  ASSERT_EQ(actualInstance1, instance2->actual())
      << "delete instance1 should not destroy the actual filter instance";
  delete instance2;  // Rime session 2 ends

  the<Engine> engine2(Engine::Create());
  Ticket ticket2(engine2.get(), "test_namespace", "test");
  auto* instance3 = component.Create(ticket2);
  ASSERT_EQ(actualInstance1, instance3->actual())
      << "delete instance1 should not destroy the actual filter instance";
  delete instance3;  // Rime session 3 ends
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TYPED_TEST(QuickJSComponentTest, CreateComponent) {
  QuickJSComponent<MockFilter<TypeParam>, Filter, TypeParam> component;

  the<Engine> engine1(Engine::Create());
  Ticket ticket(engine1.get(), "test_namespace", "test");
  auto* instance1 = component.Create(ticket);
  ASSERT_NE(nullptr, instance1);

  auto* instance2 = component.Create(ticket);
  ASSERT_EQ(instance1->actual(), instance2->actual())
      << "should return the same actual filter with the same ticket";

  the<Engine> engine2(Engine::Create());
  Ticket ticket2(engine2.get(), "test_namespace", "test");
  auto* instance3 = component.Create(ticket2);
  ASSERT_EQ(instance1->actual(), instance3->actual())
      << "should return the same actual filter with the same ticket namespace";

  the<Engine> engine3(Engine::Create());
  Ticket ticket3(engine3.get(), "test_namespace2", "test");
  auto* instance4 = component.Create(ticket3);
  ASSERT_TRUE(instance4 != nullptr);
  ASSERT_NE(instance1->actual(), instance4->actual())
      << "should create a new instance with a different ticket namespace";

  delete instance4;
  delete instance3;
  delete instance2;
  delete instance1;
}
