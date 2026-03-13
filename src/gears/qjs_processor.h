#pragma once

#include <rime/processor.h>

#include <rime/gear/translator_commons.h>
#include <rime/translation.h>
#include "qjs_component.h"
#include "qjs_module.h"

template <typename T_JS_VALUE>
class QuickJSProcessor : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSProcessor(const rime::Ticket& ticket, Environment* environment);

  rime::ProcessResult processKeyEvent(const rime::KeyEvent& keyEvent, Environment* environment);
};

// Specialization for Processor
namespace rime {
template <typename T_ACTUAL, typename T_JS_VALUE>
class ComponentWrapper<T_ACTUAL, rime::Processor, T_JS_VALUE>
    : public ComponentWrapperBase<T_ACTUAL, rime::Processor, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const rime::Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, rime::Processor, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  rime::ProcessResult ProcessKeyEvent(const rime::KeyEvent& keyEvent) override {
    return this->actual()->processKeyEvent(keyEvent, this->environment());
  }
};
}  // namespace rime
