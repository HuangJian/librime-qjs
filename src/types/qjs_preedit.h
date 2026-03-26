#pragma once

#include <rime/composition.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Preedit> {
public:
  EXPORT_CLASS_WITH_SHARED_POINTER(Preedit,
                                   WITH_CONSTRUCTOR(),
                                   WITH_PROPERTIES(AUTO_PROPERTIES(text,
                                                                   (caretPos, caret_pos),
                                                                   (selectStart, sel_start),
                                                                   (selectEnd, sel_end))),
                                   WITH_GETTERS(),
                                   WITH_FUNCTIONS());
};
