#pragma once

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "misc/system_info.h"

template <>
class JsWrapper<SystemInfo> {
public:
  EXPORT_CLASS_WITH_RAW_POINTER(SystemInfo,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS((name, obj->getOSName()),
                                             (version, obj->getOSVersion()),
                                             (architecture, obj->getArchitecture())),
                                WITHOUT_FUNCTIONS);
};
