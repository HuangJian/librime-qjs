#pragma once

#include <rime/key_event.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<KeyEvent> {
  DEFINE_GETTER(KeyEvent, shift, obj->shift())
  DEFINE_GETTER(KeyEvent, ctrl, obj->ctrl())
  DEFINE_GETTER(KeyEvent, alt, obj->alt())
  DEFINE_GETTER(KeyEvent, release, obj->release())
  DEFINE_GETTER(KeyEvent, repr, obj->repr())

public:
  EXPORT_CLASS_WITH_RAW_POINTER(KeyEvent,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS(shift, ctrl, alt, release, repr),
                                WITHOUT_FUNCTIONS);
};
