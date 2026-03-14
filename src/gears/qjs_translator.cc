#include "qjs_translator.h"
#include <glog/logging.h>

template <typename T_JS_VALUE>
QuickJSTranslator<T_JS_VALUE>::QuickJSTranslator(const Ticket& ticket, Environment& environment)
    : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "translate") {}

template <typename T_JS_VALUE>
an<Translation> QuickJSTranslator<T_JS_VALUE>::query(const std::string& input,
                                                     const Segment& segment,
                                                     Environment& environment) {
  auto translation = New<FifoTranslation>();
  if (!this->isLoaded()) {
    return translation;
  }

  auto& engine = JsEngine<T_JS_VALUE>::instance();
  T_JS_VALUE jsInput = engine.wrap(input);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  T_JS_VALUE jsSegment = engine.wrap(const_cast<Segment*>(&segment));
  auto jsEnvironment = engine.wrap(&environment);
  T_JS_VALUE args[] = {jsInput, jsSegment, jsEnvironment};
  T_JS_VALUE resultArray =
      engine.callFunction(this->getMainFunc(), this->getInstance(), countof(args), args);
  engine.freeValue(jsInput, jsSegment, jsEnvironment);
  if (!engine.isArray(resultArray)) {
    LOG(ERROR) << "[qjs] A candidate array should be returned by `translate` of the plugin: "
               << this->getNamespace();
    return translation;
  }

  const size_t length = engine.getArrayLength(resultArray);
  for (uint32_t i = 0; i < length; i++) {
    T_JS_VALUE item = engine.getArrayItem(resultArray, i);
    if (const an<Candidate> candidate = engine.template unwrap<Candidate>(item)) {
      translation->Append(candidate);
    } else {
      LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
    }
    engine.freeValue(item);
  }

  engine.freeValue(resultArray);
  return translation;
}

namespace rime {

template class ComponentWrapper<QuickJSTranslator<JSValue>, Translator, JSValue>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class ComponentWrapper<QuickJSTranslator<JSValueRef>, Translator, JSValueRef>;
#endif
}  // namespace rime

template class QuickJSTranslator<JSValue>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSTranslator<JSValueRef>;
#endif
