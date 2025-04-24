#pragma once

#include <rime/common.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using NotifierConnection = rime::connection;

constexpr const char* JS_LISTENER_PROPERTY_NAME = "jsListenerFunc";

template <>
class JsWrapper<NotifierConnection> {
  DEFINE_CFUNCTION(disconnect, {
    auto obj = engine.unwrap<NotifierConnection>(thisVal);
    obj->disconnect();
    auto jsListenerFunc = engine.getObjectProperty(thisVal, JS_LISTENER_PROPERTY_NAME);
    engine.freeValue(jsListenerFunc);
    return engine.undefined();
  })

  DEFINE_GETTER(NotifierConnection, isConnected, obj->connected())

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(NotifierConnection,
                                   WITHOUT_CONSTRUCTOR,
                                   WITHOUT_PROPERTIES,
                                   WITH_GETTERS(isConnected),
                                   WITH_FUNCTIONS(disconnect, 0));
};
