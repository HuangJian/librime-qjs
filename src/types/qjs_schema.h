#pragma once

#include <rime/schema.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Schema> {
public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(
      Schema,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(),
      JS_API_WITH_GETTERS((id, obj->schema_id()),
                          (name, obj->schema_name()),
                          config,  // equal to `(config, obj->config())`
                          (pageSize, obj->page_size()),
                          (selectKeys, obj->select_keys())),
      JS_API_WITH_FUNCTIONS());
};
