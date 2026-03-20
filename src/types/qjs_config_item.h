#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigItem> {
  DEFINE_CFUNCTION(getType, {
    auto obj = engine.unwrap<rime::ConfigItem>(thisVal);
    const char* strType;
    switch (obj->type()) {
      case rime::ConfigItem::kNull:
        strType = "null";
        break;
      case rime::ConfigItem::kScalar:
        strType = "scalar";
        break;
      case rime::ConfigItem::kList:
        strType = "list";
        break;
      case rime::ConfigItem::kMap:
        strType = "map";
        break;
      default:
        strType = "unknown";
    }
    return engine.wrap(strType);
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(ConfigItem,
                                   WITHOUT_CONSTRUCTOR,
                                   WITHOUT_PROPERTIES,
                                   WITHOUT_GETTERS,
                                   WITH_FUNCTIONS(getType));
};
