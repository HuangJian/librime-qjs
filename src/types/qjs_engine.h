#pragma once

#include <rime/engine.h>
#include <rime/key_event.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_schema.h"

using namespace rime;

template <>
class JsWrapper<rime::Engine> {
  DEFINE_GETTER(Engine, schema, obj->schema())
  DEFINE_GETTER(Engine, context, obj->context())
  DEFINE_GETTER(Engine, activeEngine, obj->active_engine())

  DEFINE_CFUNCTION_ARGC(commitText, 1, {
    std::string text = engine.toStdString(argv[0]);
    auto* obj = engine.unwrap<Engine>(thisVal);
    obj->CommitText(text);
    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(applySchema, 1, {
    auto schema = engine.unwrap<Schema>(argv[0]);
    if (!schema) {
      return engine.jsFalse();
    }
    auto* obj = engine.unwrap<Engine>(thisVal);
    obj->ApplySchema(schema);
    return engine.jsTrue();
  })

  DEFINE_CFUNCTION_ARGC(processKey, 1, {
    std::string keyRepr = engine.toStdString(argv[0]);
    auto* obj = engine.unwrap<Engine>(thisVal);
    return engine.wrap(obj->ProcessKey(KeyEvent(keyRepr)));
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Engine,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS(schema, context, activeEngine),
                                WITH_FUNCTIONS(processKey, 1, commitText, 1, applySchema, 1));
};
