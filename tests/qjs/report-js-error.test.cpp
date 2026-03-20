#include <gtest/gtest.h>
#include <quickjs.h>

#include <string>

#include "../test_switch.h"

std::string trim(const std::string& str) {
  const auto start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }

  const auto end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

template <typename T>
class QuickJSErrorTest : public ::testing::Test {};

SETUP_JS_ENGINES(QuickJSErrorTest);

TYPED_TEST(QuickJSErrorTest, TestJsRuntimeError) {
  auto& jsEngine = JsEngine<QjsValueRAII>::instance();
  auto module = jsEngine.loadJsFile("runtime-error.js");
  auto globalObj = jsEngine.getGlobalObject();
  auto func = jsEngine.getObjectProperty(globalObj, "funcWithRuntimeError");
  ASSERT_TRUE(jsEngine.isFunction(func));

  auto result = jsEngine.callFunction(func, JS_UNDEFINED, 0, nullptr);
  ASSERT_TRUE(jsEngine.isException(result));

  // The exception is alredy captured in the js engine logger. The log should be:
  // ReferenceError: abcdefg is not defined
  //      at <anonymous> (runtime-error.js:7:21)
}
