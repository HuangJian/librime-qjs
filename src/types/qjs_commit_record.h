#pragma once

#include <rime/commit_history.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<CommitRecord> {
public:
  EXPORT_CLASS_WITH_RAW_POINTER(CommitRecord,
                                WITH_CONSTRUCTOR(),
                                WITH_PROPERTIES(AUTO_PROPERTIES(text, type)),
                                WITH_GETTERS(),
                                WITH_FUNCTIONS());
};
