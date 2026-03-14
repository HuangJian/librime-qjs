#include "qjs_processor.h"
#include <glog/logging.h>

template <typename T_JS_VALUE>
QuickJSProcessor<T_JS_VALUE>::QuickJSProcessor(const Ticket& ticket, Environment* environment)
    : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "process") {}

template <typename T_JS_VALUE>
ProcessResult QuickJSProcessor<T_JS_VALUE>::processKeyEvent(const KeyEvent& keyEvent,
                                                            Environment* environment) {
  if (!this->isLoaded()) {
    return kNoop;
  }

  auto& engine = JsEngine<T_JS_VALUE>::instance();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  T_JS_VALUE jsKeyEvt = engine.wrap(const_cast<KeyEvent*>(&keyEvent));
  auto jsEnvironment = engine.wrap(environment);
  T_JS_VALUE args[2];
  args[0] = std::move(jsKeyEvt);
  args[1] = std::move(jsEnvironment);
  T_JS_VALUE jsResult = engine.callFunction(this->getMainFunc(), this->getInstance(), 2, args);
  engine.freeValue(args[0], args[1]);

  if (engine.isException(jsResult)) {
    LOG(ERROR) << "[qjs] " << this->getNamespace()
               << " failed to process keyEvent = " << keyEvent.repr();
    return kNoop;
  }

  std::string result = engine.toStdString(jsResult);
  engine.freeValue(jsResult);

  if (result == "kNoop") {
    return kNoop;
  }
  if (result == "kAccepted") {
    return kAccepted;
  }
  if (result == "kRejected") {
    return kRejected;
  }

  LOG(ERROR) << "[qjs] " << this->getNamespace() << "::ProcessKeyEvent unknown result: " << result;
  return kNoop;
}

namespace rime {
template class ComponentWrapper<QuickJSProcessor<QjsValueRAII>, Processor, QjsValueRAII>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class ComponentWrapper<QuickJSProcessor<JSValueRef>, Processor, JSValueRef>;
#endif
}  // namespace rime

template class QuickJSProcessor<QjsValueRAII>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSProcessor<JSValueRef>;
#endif
