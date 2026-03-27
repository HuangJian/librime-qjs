#pragma once

#include <rime/key_event.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<KeyEvent> {
public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(KeyEvent,
                                       JS_API_WITH_CONSTRUCTOR(),
                                       JS_API_WITH_PROPERTIES(),
                                       JS_API_WITH_GETTERS(shift, ctrl, alt, release, repr),
                                       JS_API_WITH_FUNCTIONS());
};
