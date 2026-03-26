#pragma once

#include <rime/schema.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Schema> {
public:
  EXPORT_CLASS_WITH_RAW_POINTER(Schema,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS((id, obj->schema_id()),
                                             (name, obj->schema_name()),
                                             config,
                                             (pageSize, obj->page_size()),
                                             (selectKeys, obj->select_keys())),
                                WITHOUT_FUNCTIONS);
};
