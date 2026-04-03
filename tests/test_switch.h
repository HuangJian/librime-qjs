#pragma once

#include "engines/common.h"

#ifdef _ENABLE_JAVASCRIPTCORE

// GoogleTest typed-test registration is macro-based, so this wrapper must be too.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SETUP_JS_ENGINES(testSuite)                      \
  using JsTypes = ::testing::Types<JSValue, JSValueRef>; \
  TYPED_TEST_SUITE(testSuite, JsTypes);

#else

// GoogleTest typed-test registration is macro-based, so this wrapper must be too.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SETUP_JS_ENGINES(testSuite)          \
  using JsTypes = ::testing::Types<JSValue>; \
  TYPED_TEST_SUITE(testSuite, JsTypes);

#endif
