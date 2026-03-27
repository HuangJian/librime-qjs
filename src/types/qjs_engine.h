#pragma once

#include <rime/engine.h>
#include <rime/key_event.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_schema.h"

using namespace rime;

template <>
class JsWrapper<Engine> {
  JS_API_DEFINE_CFUNCTION_ARGC(commitText, 1, {
    std::string text = engine.toStdString(argv[0]);
    obj->CommitText(text);
    return engine.undefined();
  })

  JS_API_DEFINE_CFUNCTION_ARGC(applySchema, 1, {
    auto schema = engine.unwrap<Schema>(argv[0]);
    if (!schema) {
      return engine.jsFalse();
    }
    obj->ApplySchema(schema);
    return engine.jsTrue();
  })

  JS_API_DEFINE_CFUNCTION_ARGC(processKey, 1, {
    std::string keyRepr = engine.toStdString(argv[0]);
    return engine.wrap(obj->ProcessKey(KeyEvent(keyRepr)));
  })

public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(Engine,
                                       JS_API_WITH_CONSTRUCTOR(),
                                       JS_API_WITH_PROPERTIES(),
                                       JS_API_WITH_GETTERS(schema,
                                                           context,
                                                           (activeEngine, obj->active_engine())),
                                       JS_API_WITH_FUNCTIONS(processKey, commitText, applySchema));
};
