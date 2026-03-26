#pragma once

#include <rime/composition.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Preedit> {
  DEFINE_GETTER(Preedit, text, obj->text)
  DEFINE_GETTER(Preedit, caretPos, obj->caret_pos)
  DEFINE_GETTER(Preedit, selectStart, obj->sel_start)
  DEFINE_GETTER(Preedit, selectEnd, obj->sel_end)

  DEFINE_SETTER(Preedit, text, obj->text = value)
  DEFINE_SETTER(Preedit, caretPos, obj->caret_pos = value)
  DEFINE_SETTER(Preedit, selectStart, obj->sel_start = value)
  DEFINE_SETTER(Preedit, selectEnd, obj->sel_end = value)

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(Preedit,
                                   WITHOUT_CONSTRUCTOR,
                                   WITH_PROPERTIES(text, caretPos, selectStart, selectEnd),
                                   WITHOUT_GETTERS,
                                   WITHOUT_FUNCTIONS);
};
