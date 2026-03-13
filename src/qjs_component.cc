#include "qjs_component.h"
#include <glog/logging.h>
#include "gears/qjs_filter.h"
#include "gears/qjs_processor.h"
#include "gears/qjs_translator.h"

namespace rime {

template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
ComponentWrapperBase<T_ACTUAL, T_BASE, T_JS_VALUE>::ComponentWrapperBase(const rime::Ticket& ticket)
    : T_BASE(ticket),
      environment_(std::make_unique<Environment>(ticket.engine, ticket.name_space)) {
  DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name()
             << " ComponentWrapper created with ticket: " << ticket.name_space;
}

template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
ComponentWrapperBase<T_ACTUAL, T_BASE, T_JS_VALUE>::~ComponentWrapperBase() {
  DLOG(INFO) << "[qjs] " << typeid(T_ACTUAL).name() << " ComponentWrapper destroyed";
}

template <typename T_ACTUAL, typename T_BASE, typename T_JS_VALUE>
ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>*
QuickJSComponent<T_ACTUAL, T_BASE, T_JS_VALUE>::Create(const rime::Ticket& ticket) {
  // The same plugin could have difference configurations for different schemas, and then behave differently.
  // So we need to create a new component for each schema.
  const std::string schemaId = ticket.engine->schema()->schema_id();
  KeyType key = std::make_pair(schemaId, ticket.name_space);

  auto component = new ComponentWrapper<T_ACTUAL, T_BASE, T_JS_VALUE>(ticket);
  std::shared_ptr<T_ACTUAL> actual = nullptr;
  if (components_.count(key)) {
    actual = components_[key];
  } else {
    LOG(INFO) << "[qjs] creating component '" << ticket.name_space << "' for schema " << schemaId;
    actual = std::make_shared<T_ACTUAL>(ticket, component->environment());
    components_[key] = actual;
  }

  component->setActual(actual);
  return component;
}

// Explicit instantiations for the types used in module.cc
template class QuickJSComponent<QuickJSProcessor<JSValue>, Processor, JSValue>;
template class QuickJSComponent<QuickJSFilter<JSValue>, Filter, JSValue>;
template class QuickJSComponent<QuickJSTranslator<JSValue>, Translator, JSValue>;

#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSComponent<QuickJSProcessor<JSValueRef>, Processor, JSValueRef>;
template class QuickJSComponent<QuickJSFilter<JSValueRef>, Filter, JSValueRef>;
template class QuickJSComponent<QuickJSTranslator<JSValueRef>, Translator, JSValueRef>;
#endif

}  // namespace rime
