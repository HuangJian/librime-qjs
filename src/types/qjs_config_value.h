#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigValue> {
  JS_API_DEFINE_CFUNCTION(getType, { return engine.wrap("scalar"); })

  JS_API_DEFINE_CFUNCTION(getBool, {
    bool value = false;
    bool success = obj->GetBool(&value);
    return success ? engine.wrap(value) : engine.null();
  })

  JS_API_DEFINE_CFUNCTION(getInt, {
    int value = 0;
    bool success = obj->GetInt(&value);
    return success ? engine.wrap(value) : engine.null();
  })

  JS_API_DEFINE_CFUNCTION(getDouble, {
    double value = 0;
    bool success = obj->GetDouble(&value);
    return success ? engine.wrap(value) : engine.null();
  })

  JS_API_DEFINE_CFUNCTION(getString, {
    std::string value;
    bool success = obj->GetString(&value);
    return success ? engine.wrap(value.c_str()) : engine.null();
  })

public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(
      ConfigValue,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(),
      JS_API_WITH_GETTERS(),
      JS_API_WITH_FUNCTIONS(getType, getBool, getInt, getDouble, getString));
};
