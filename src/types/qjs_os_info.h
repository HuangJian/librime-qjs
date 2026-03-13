#pragma once

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "misc/system_info.h"

template <>
class JsWrapper<SystemInfo> {
  DEFINE_GETTER(SystemInfo, name, obj->getOSName());
  DEFINE_GETTER(SystemInfo, version, obj->getOSVersion());
  DEFINE_GETTER(SystemInfo, architecture, obj->getArchitecture());

public:
  EXPORT_CLASS_WITH_RAW_POINTER(SystemInfo,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS(name, version, architecture),
                                WITHOUT_FUNCTIONS);
};
