#pragma once

#include <rime/candidate.h>
#include <rime/gear/translator_commons.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

using CandidateIterator = rime::Translation;

template <>
class JsWrapper<rime::Translation> {
  DEFINE_CFUNCTION(next, {
    auto obj = engine.unwrap<CandidateIterator>(thisVal);
    if (obj->exhausted()) {
      return engine.null();
    }
    auto candidate = obj->Peek();
    obj->Next();
    return engine.wrap(candidate);
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(CandidateIterator,
                                   WITHOUT_CONSTRUCTOR,
                                   WITHOUT_PROPERTIES,
                                   WITHOUT_GETTERS,
                                   WITH_FUNCTIONS(next, 0));
};
