#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigValue> {
  DEFINE_CFUNCTION(getType, { return engine.wrap("scalar"); })

  DEFINE_CFUNCTION(getBool, {
    auto obj = engine.unwrap<rime::ConfigValue>(thisVal);
    bool value = false;
    bool success = obj->GetBool(&value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION(getInt, {
    auto obj = engine.unwrap<rime::ConfigValue>(thisVal);
    int value = 0;
    bool success = obj->GetInt(&value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION(getDouble, {
    auto obj = engine.unwrap<rime::ConfigValue>(thisVal);
    double value = 0;
    bool success = obj->GetDouble(&value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION(getString, {
    auto obj = engine.unwrap<rime::ConfigValue>(thisVal);
    std::string value;
    bool success = obj->GetString(&value);
    return success ? engine.wrap(value.c_str()) : engine.null();
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      ConfigValue,
      WITHOUT_CONSTRUCTOR,
      WITHOUT_PROPERTIES,
      WITHOUT_GETTERS,
      WITH_FUNCTIONS(getType, getBool, getInt, getDouble, getString));
};
