#pragma once

#include <rime/gear/translator_commons.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

using CandidateIterator = Translation;

template <>
class JsWrapper<Translation> {
  JS_API_DEFINE_CFUNCTION(next, {
    if (obj->exhausted()) {
      return engine.null();
    }
    auto candidate = obj->Peek();
    obj->Next();
    return engine.wrap(candidate);
  })

public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(CandidateIterator,
                                          JS_API_WITH_CONSTRUCTOR(),
                                          JS_API_WITH_PROPERTIES(),
                                          JS_API_WITH_GETTERS(),
                                          JS_API_WITH_FUNCTIONS(next));
};
