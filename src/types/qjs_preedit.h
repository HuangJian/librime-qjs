#pragma once

#include <rime/composition.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<rime::Preedit> {
  DEFINE_GETTER(Preedit, text, obj->text)
  DEFINE_GETTER(Preedit, caretPos, obj->caret_pos)
  DEFINE_GETTER(Preedit, selectStart, obj->sel_start)
  DEFINE_GETTER(Preedit, selectEnd, obj->sel_end)

  DEFINE_STRING_SETTER(Preedit, text, obj->text = str)
  DEFINE_SETTER(Preedit, caretPos, engine.toInt, obj->caret_pos = value)
  DEFINE_SETTER(Preedit, selectStart, engine.toInt, obj->sel_start = value)
  DEFINE_SETTER(Preedit, selectEnd, engine.toInt, obj->sel_end = value)

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(Preedit,
                                   WITHOUT_CONSTRUCTOR,
                                   WITH_PROPERTIES(text, caretPos, selectStart, selectEnd),
                                   WITHOUT_GETTERS,
                                   WITHOUT_FUNCTIONS);
};
