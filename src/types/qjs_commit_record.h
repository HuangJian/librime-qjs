#pragma once

#include <rime/commit_history.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<CommitRecord> {
  DEFINE_GETTER(CommitRecord, type, obj->type)
  DEFINE_GETTER(CommitRecord, text, obj->text)

  DEFINE_STRING_SETTER(CommitRecord, text, obj->text = str)
  DEFINE_STRING_SETTER(CommitRecord, type, obj->type = str)

public:
  EXPORT_CLASS_WITH_RAW_POINTER(CommitRecord,
                                WITHOUT_CONSTRUCTOR,
                                WITH_PROPERTIES(text, type),
                                WITHOUT_GETTERS,
                                WITHOUT_FUNCTIONS);
};
