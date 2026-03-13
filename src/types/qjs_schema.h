#pragma once

#include <rime/schema.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Schema> {
  DEFINE_GETTER(Schema, id, obj->schema_id())
  DEFINE_GETTER(Schema, name, obj->schema_name())
  DEFINE_GETTER(Schema, config, obj->config())
  DEFINE_GETTER(Schema, pageSize, obj->page_size())
  DEFINE_GETTER(Schema, selectKeys, obj->select_keys())

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Schema,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS(id, name, config, pageSize, selectKeys),
                                WITHOUT_FUNCTIONS);
};
