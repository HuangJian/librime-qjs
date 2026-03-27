#pragma once

#include <rime/context.h>

#include "engines/js_macros.h"
#include "js_exception.h"
#include "js_wrapper.h"
#include "qjs_notifier_connection.h"

using Notifier = rime::signal<void(Context*)>;
using NotifierConnection = connection;

template <>
class JsWrapper<Notifier> {
  template <typename T>  // <-- make it a template function to satisfy the clang compiler
  static void handleNotification(JsEngine<T>& engine, const T& jsFunc, Context* rimeContext) {
    auto undefined = engine.toObject(engine.undefined());
    T arg = engine.wrap(rimeContext);
    auto result = engine.callFunction(engine.toObject(jsFunc), undefined, 1, &arg);
    if (engine.isException(result)) {
      LOG(ERROR) << "Error in notifying the js connection";
    }
    engine.freeValue(result, arg);
  }

  JS_API_DEFINE_CFUNCTION_ARGC(connect, 1, {
    auto jsListenerFunc = argv[0];
    if (!engine.isFunction(jsListenerFunc)) {
      const char* msg = "The argument of notifier.connect(arg) should be a function";
      throw new JsException(JsErrorType::TYPE, msg);
    }

    // IMPORTANT: jsListenerFunc should be duplicated before passing to JS_Call,
    // otherwise it will be released by the quickjs engine and the function will not be called
    auto duplicatedFunc = engine.duplicateValue(jsListenerFunc);

    auto connection = std::make_shared<NotifierConnection>(
        // NOTE: duplicatedFunc should be passed by value but not by reference "&duplicatedFunc",
        // otherwise it could crash the program when running with the JavaScriptCore engine.
        // I guess it could have been released in jsc's garbage collection.
        obj->connect([&engine, duplicatedFunc](rime::Context* rimeContext) {
          handleNotification(engine, duplicatedFunc, rimeContext);
        }));

    auto jsConnection = engine.wrap(connection);
    // attach it to the connection to free it by the js engine when disconnecting
    engine.setObjectProperty(jsConnection, JS_LISTENER_PROPERTY_NAME, duplicatedFunc);
    return jsConnection;
  })

public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(Notifier,
                                       JS_API_WITH_CONSTRUCTOR(),
                                       JS_API_WITH_PROPERTIES(),
                                       JS_API_WITH_GETTERS(),
                                       JS_API_WITH_FUNCTIONS(connect));
};
