#pragma once

#include <rime/commit_history.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<CommitRecord> {
public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(CommitRecord,
                                       JS_API_WITH_CONSTRUCTOR(),
                                       JS_API_WITH_PROPERTIES(JS_API_AUTO_PROPERTIES(text, type)),
                                       JS_API_WITH_GETTERS(),
                                       JS_API_WITH_FUNCTIONS());
};
