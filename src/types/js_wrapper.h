#pragma once

#include <quickjs.h>
#include <memory>

template <typename T_RIME_TYPE>
class JsWrapper {
public:
  // to satisfy clang-tidy -_-!
  using T_UNWRAP_TYPE = std::shared_ptr<T_RIME_TYPE>;
  inline static auto typeName = "Unknown";
  inline static JSClassID jsClassId = 0;
};
