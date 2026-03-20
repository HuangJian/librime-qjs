#include "qjs_translator.h"
#include <glog/logging.h>

template <typename T_JS_VALUE>
QuickJSTranslator<T_JS_VALUE>::QuickJSTranslator(const Ticket& ticket,
                                                 const Environment* environment)
    : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "translate") {}

template <typename T_JS_VALUE>
an<Translation> QuickJSTranslator<T_JS_VALUE>::query(const std::string& input,
                                                     const Segment& segment,
                                                     const Environment* environment) {
  if (!this->isLoaded()) {
    return nullptr;
  }

  auto& engine = JsEngine<T_JS_VALUE>::instance();
  auto jsInput = engine.wrap(input);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  auto jsSegment = engine.wrap(const_cast<Segment*>(&segment));
  auto jsEnv = engine.wrap(environment);
  T_JS_VALUE args[3];
  args[0] = std::move(jsInput);
  args[1] = std::move(jsSegment);
  args[2] = std::move(jsEnv);

  T_JS_VALUE jsResult = engine.callFunction(this->getMainFunc(), this->getInstance(), 3, args);

  if (engine.isException(jsResult)) {
    LOG(ERROR) << "[qjs] Exception thrown while querying translator " << this->getNamespace();
    return nullptr;
  }

  if (auto translation = engine.template unwrap<Translation>(jsResult)) {
    return translation;
  }

  // fallback to a generator based approach if the return value is not a Translation
  return std::make_shared<QuickJSFastTranslation<T_JS_VALUE>>(nullptr, this->getInstance(),
                                                              this->getMainFunc(), environment);
}

namespace rime {
template class ComponentWrapper<QuickJSTranslator<QjsValueRAII>, Translator, QjsValueRAII>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class ComponentWrapper<QuickJSTranslator<JSValueRef>, Translator, JSValueRef>;
#endif
}  // namespace rime

template class QuickJSTranslator<QjsValueRAII>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSTranslator<JSValueRef>;
#endif
