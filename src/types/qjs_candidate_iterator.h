#pragma once

#include <rime/gear/translator_commons.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

using CandidateIterator = Translation;

template <>
class JsWrapper<Translation> {
  DEFINE_CFUNCTION(next, {
    if (obj->exhausted()) {
      return engine.null();
    }
    auto candidate = obj->Peek();
    obj->Next();
    return engine.wrap(candidate);
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(CandidateIterator,
                                   WITH_CONSTRUCTOR(),
                                   WITH_PROPERTIES(),
                                   WITH_GETTERS(),
                                   WITH_FUNCTIONS(next));
};
