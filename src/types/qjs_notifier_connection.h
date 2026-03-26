#pragma once

#include <rime/common.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using NotifierConnection = connection;

constexpr const char* JS_LISTENER_PROPERTY_NAME = "jsListenerFunc";

template <>
class JsWrapper<NotifierConnection> {
  DEFINE_CFUNCTION(disconnect, {
    obj->disconnect();
    auto jsListenerFunc = engine.getObjectProperty(thisVal, JS_LISTENER_PROPERTY_NAME);
    engine.freeValue(jsListenerFunc);
    return engine.undefined();
  })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(NotifierConnection,
                                   WITHOUT_CONSTRUCTOR,
                                   WITHOUT_PROPERTIES,
                                   WITH_GETTERS((isConnected, obj->connected())),
                                   WITH_FUNCTIONS(disconnect));
};
