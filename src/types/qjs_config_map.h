#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigMap> {
  JS_API_DEFINE_CFUNCTION(getType, { return engine.wrap("map"); })

  JS_API_DEFINE_CFUNCTION_ARGC(hasKey, 1, {
    auto key = engine.toStdString(argv[0]);
    return engine.wrap(obj->HasKey(key));
  })

  JS_API_DEFINE_CFUNCTION_ARGC(getItem, 1, {
    auto key = engine.toStdString(argv[0]);
    auto value = obj->Get(key);
    if (!value) {
      return engine.null();
    }
    return engine.wrap(value);
  })

  JS_API_DEFINE_CFUNCTION_ARGC(getValue, 1, {
    auto key = engine.toStdString(argv[0]);
    auto value = obj->GetValue(key);
    if (!value) {
      return engine.null();
    }
    return engine.wrap(value);
  })

  JS_API_DEFINE_CFUNCTION_ARGC(setItem, 2, {
    auto key = engine.toStdString(argv[0]);
    if (auto item = engine.unwrap<rime::ConfigItem>(argv[1])) {
      obj->Set(key, item);
    }
    return engine.undefined();
  })

public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(
      ConfigMap,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(),
      JS_API_WITH_GETTERS(),
      JS_API_WITH_FUNCTIONS(getType, hasKey, getItem, getValue, setItem));
};
