#pragma once

#include <rime/composition.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Preedit> {
public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      Preedit,
      WITHOUT_CONSTRUCTOR,
      WITH_PROPERTIES(
          AUTO_PROPERTIES(text),
          AUTO_PROPERTIES_RENAMED(caretPos, caret_pos, selectStart, sel_start, selectEnd, sel_end)),
      WITHOUT_GETTERS,
      WITHOUT_FUNCTIONS);
};
