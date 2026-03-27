#pragma once

#include <rime/commit_history.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<CommitHistory> {
  JS_API_DEFINE_CFUNCTION_ARGC(push, 2, {
    auto type = engine.toStdString(argv[0]);
    auto text = engine.toStdString(argv[1]);
    obj->Push(CommitRecord(type, text));
    return engine.undefined();
  })

public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(
      CommitHistory,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(),
      JS_API_WITH_GETTERS((last, obj->empty() ? nullptr : &obj->back()), repr),
      JS_API_WITH_FUNCTIONS(push));
};
