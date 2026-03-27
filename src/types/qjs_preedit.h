#pragma once

#include <rime/composition.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Preedit> {
public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(
      Preedit,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(JS_API_AUTO_PROPERTIES(text,
                                                    (caretPos, caret_pos),
                                                    (selectStart, sel_start),
                                                    (selectEnd, sel_end))),
      JS_API_WITH_GETTERS(),
      JS_API_WITH_FUNCTIONS());
};
