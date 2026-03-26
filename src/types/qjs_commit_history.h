#pragma once

#include <rime/commit_history.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<CommitHistory> {
  DEFINE_CFUNCTION_ARGC(push, 2, {
    auto type = engine.toStdString(argv[0]);
    auto text = engine.toStdString(argv[1]);
    auto* obj = engine.unwrap<rime::CommitHistory>(thisVal);
    obj->Push(CommitRecord(type, text));
    return engine.undefined();
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(CommitHistory,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS((last, obj->empty() ? nullptr : &obj->back()), repr),
                                WITH_FUNCTIONS(push));
};
