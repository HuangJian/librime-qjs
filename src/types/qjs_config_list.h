#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigList> {
  JS_API_DEFINE_CFUNCTION(getType, { return engine.wrap("list"); })

  JS_API_DEFINE_CFUNCTION(getSize, { return engine.wrap(obj->size()); })

  JS_API_DEFINE_CFUNCTION_ARGC(getItemAt, 1, {
    int index = engine.toInt(argv[0]);

    if (index < 0 || size_t(index) >= obj->size()) {
      return engine.null();
    }

    auto item = obj->GetAt(index);
    if (!item) {
      return engine.null();
    }
    return engine.wrap(item);
  })

  JS_API_DEFINE_CFUNCTION_ARGC(getValueAt, 1, {
    int index = engine.toInt(argv[0]);

    if (index < 0 || size_t(index) >= obj->size()) {
      return engine.null();
    }

    auto value = obj->GetValueAt(index);
    if (!value) {
      return engine.null();
    }
    return engine.wrap(value);
  })

  JS_API_DEFINE_CFUNCTION_ARGC(pushBack, 1, {
    if (auto item = engine.unwrap<ConfigItem>(argv[0])) {
      obj->Append(item);
    }
    return engine.undefined();
  })

  JS_API_DEFINE_CFUNCTION(clear, {
    obj->Clear();
    return engine.undefined();
  })

public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(
      ConfigList,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(),
      JS_API_WITH_GETTERS(),
      JS_API_WITH_FUNCTIONS(getType, getSize, getItemAt, getValueAt, pushBack, clear));
};
