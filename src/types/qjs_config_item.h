#pragma once

#include <rime/config/config_types.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<ConfigItem> {
  JS_API_DEFINE_CFUNCTION(getType, {
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
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(ConfigItem,
                                          JS_API_WITH_CONSTRUCTOR(),
                                          JS_API_WITH_PROPERTIES(),
                                          JS_API_WITH_GETTERS(),
                                          JS_API_WITH_FUNCTIONS(getType));
};
