#include "qjs_processor.h"
#include <glog/logging.h>

template <typename T_JS_VALUE>
QuickJSProcessor<T_JS_VALUE>::QuickJSProcessor(const rime::Ticket& ticket, Environment* environment)
    : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "process") {}

template <typename T_JS_VALUE>
rime::ProcessResult QuickJSProcessor<T_JS_VALUE>::processKeyEvent(const rime::KeyEvent& keyEvent,
                                                                  Environment* environment) {
  if (!this->isLoaded()) {
    return rime::kNoop;
  }

  auto& engine = JsEngine<T_JS_VALUE>::instance();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  T_JS_VALUE jsKeyEvt = engine.wrap(const_cast<rime::KeyEvent*>(&keyEvent));
  auto jsEnvironment = engine.wrap(environment);
  T_JS_VALUE args[] = {jsKeyEvt, jsEnvironment};
  T_JS_VALUE jsResult = engine.callFunction(this->getMainFunc(), this->getInstance(), 2, args);
  engine.freeValue(jsKeyEvt, jsEnvironment);

  if (engine.isException(jsResult)) {
    LOG(ERROR) << "[qjs] " << this->getNamespace()
               << " failed to process keyEvent = " << keyEvent.repr();
    return rime::kNoop;
  }

  std::string result = engine.toStdString(jsResult);
  engine.freeValue(jsResult);

  if (result == "kNoop") {
    return rime::kNoop;
  }
  if (result == "kAccepted") {
    return rime::kAccepted;
  }
  if (result == "kRejected") {
    return rime::kRejected;
  }

  LOG(ERROR) << "[qjs] " << this->getNamespace() << "::ProcessKeyEvent unknown result: " << result;
  return rime::kNoop;
}

namespace rime {
template <typename T_ACTUAL, typename T_JS_VALUE>
rime::ProcessResult ComponentWrapper<T_ACTUAL, rime::Processor, T_JS_VALUE>::ProcessKeyEvent(
    const rime::KeyEvent& keyEvent) {
  return this->actual()->processKeyEvent(keyEvent, this->environment());
}

template class ComponentWrapper<QuickJSProcessor<JSValue>, rime::Processor, JSValue>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class ComponentWrapper<QuickJSProcessor<JSValueRef>, rime::Processor, JSValueRef>;
#endif
}  // namespace rime

template class QuickJSProcessor<JSValue>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSProcessor<JSValueRef>;
#endif
