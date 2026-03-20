#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigMap> {
  DEFINE_CFUNCTION(getType, { return engine.wrap("map"); })

  DEFINE_CFUNCTION_ARGC(hasKey, 1, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::ConfigMap>(thisVal);
    return engine.wrap(obj->HasKey(key));
  })

  DEFINE_CFUNCTION_ARGC(getItem, 1, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::ConfigMap>(thisVal);
    auto value = obj->Get(key);
    if (!value) {
      return engine.null();
    }
    return engine.wrap(value);
  })

  DEFINE_CFUNCTION_ARGC(getValue, 1, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::ConfigMap>(thisVal);
    auto value = obj->GetValue(key);
    if (!value) {
      return engine.null();
    }
    return engine.wrap(value);
  })

  DEFINE_CFUNCTION_ARGC(setItem, 2, {
    auto key = engine.toStdString(argv[0]);
    auto obj = engine.unwrap<rime::ConfigMap>(thisVal);
    if (auto item = engine.unwrap<rime::ConfigItem>(argv[1])) {
      obj->Set(key, item);
    }
    return engine.undefined();
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      ConfigMap,
      WITHOUT_CONSTRUCTOR,
      WITHOUT_PROPERTIES,
      WITHOUT_GETTERS,
      WITH_FUNCTIONS(getType, hasKey, getItem, getValue, setItem));
};
