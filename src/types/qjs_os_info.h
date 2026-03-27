#pragma once

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "misc/system_info.h"

template <>
class JsWrapper<SystemInfo> {
public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(SystemInfo,
                                       JS_API_WITH_CONSTRUCTOR(),
                                       JS_API_WITH_PROPERTIES(),
                                       JS_API_WITH_GETTERS((name, obj->getOSName()),
                                                           (version, obj->getOSVersion()),
                                                           (architecture, obj->getArchitecture())),
                                       JS_API_WITH_FUNCTIONS());
};
