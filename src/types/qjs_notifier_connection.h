#pragma once

#include <rime/common.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using NotifierConnection = connection;

constexpr auto JS_LISTENER_PROPERTY_NAME = "jsListenerFunc";

template <>
class JsWrapper<NotifierConnection> {
  JS_API_DEFINE_CFUNCTION(disconnect, {
    obj->disconnect();
    auto jsListenerFunc = engine.getObjectProperty(thisVal, JS_LISTENER_PROPERTY_NAME);
    engine.freeValue(jsListenerFunc);
    return engine.undefined();
  })

public:
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(NotifierConnection,
                                          JS_API_WITH_CONSTRUCTOR(),
                                          JS_API_WITH_PROPERTIES(),
                                          JS_API_WITH_GETTERS((isConnected, obj->connected())),
                                          JS_API_WITH_FUNCTIONS(disconnect));
};
