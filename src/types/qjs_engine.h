#pragma once

#include <rime/engine.h>
#include <rime/key_event.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_schema.h"

using namespace rime;

template <>
class JsWrapper<Engine> {
  DEFINE_CFUNCTION_ARGC(commitText, 1, {
    std::string text = engine.toStdString(argv[0]);
    obj->CommitText(text);
    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(applySchema, 1, {
    auto schema = engine.unwrap<Schema>(argv[0]);
    if (!schema) {
      return engine.jsFalse();
    }
    obj->ApplySchema(schema);
    return engine.jsTrue();
  })

  DEFINE_CFUNCTION_ARGC(processKey, 1, {
    std::string keyRepr = engine.toStdString(argv[0]);
    return engine.wrap(obj->ProcessKey(KeyEvent(keyRepr)));
  })

  DEFINE_PROPERTIES()
  DEFINE_GETTERS(schema, context, (activeEngine, obj->active_engine()))

public:
  static const JsTypeBinder<raw_ptr_type<Engine>::type>& binder() {
    static const auto b = [] {
      JsTypeBinder<raw_ptr_type<Engine>::type> binder;
#ifdef _ENABLE_JAVASCRIPTCORE
      binder.getter("schema", get_schema, get_schemaJsc);
      binder.getter("context", get_context, get_contextJsc);
      binder.getter("activeEngine", get_activeEngine, get_activeEngineJsc);

      binder.function("processKey", static_cast<int>(processKey_argc), processKey, processKeyJsc);
      binder.function("commitText", static_cast<int>(commitText_argc), commitText, commitTextJsc);
      binder.function("applySchema", static_cast<int>(applySchema_argc), applySchema,
                      applySchemaJsc);
#else
      binder.getter("schema", get_schema);
      binder.getter("context", get_context);
      binder.getter("activeEngine", get_activeEngine);

      binder.function("processKey", static_cast<int>(processKey_argc), processKey);
      binder.function("commitText", static_cast<int>(commitText_argc), commitText);
      binder.function("applySchema", static_cast<int>(applySchema_argc), applySchema);
#endif
      return binder;
    }();
    return b;
  }

  static const JSCFunctionListEntry* propertiesQjs() { return binder().propertiesQjs(); }
  static size_t propertiesSize() { return binder().propertiesSize(); }
  static const JSCFunctionListEntry* gettersQjs() { return binder().gettersQjs(); }
  static size_t gettersSize() { return binder().gettersSize(); }
  static const JSCFunctionListEntry* functionsQjs() { return binder().functionsQjs(); }
  static size_t functionsSize() { return binder().functionsSize(); }

#ifdef _ENABLE_JAVASCRIPTCORE
  static const JSStaticValue* binderPropertiesJsc() { return binder().propertiesJsc(); }
  static const JSStaticValue* binderGettersJsc() { return binder().gettersJsc(); }
  static const JSStaticFunction* binderFunctionsJsc() { return binder().functionsJsc(); }
#endif

  EXPORT_CLASS_WITH_RAW_POINTER(Engine,
                                WITH_CONSTRUCTOR(),
                                WITH_PROPERTIES(),
                                WITH_GETTERS(),
                                WITH_FUNCTIONS());
};
