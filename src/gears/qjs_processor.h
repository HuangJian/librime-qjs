#pragma once

#include <rime/processor.h>

#include "qjs_component.h"
#include "qjs_module.h"

template <typename T_JS_VALUE>
class QuickJSProcessor : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSProcessor(const Ticket& ticket, const Environment& environment);

  ProcessResult processKeyEvent(const KeyEvent& keyEvent, const Environment& environment);
};

// Specialization for Processor
template <typename T_ACTUAL, typename T_JS_VALUE>
class rime::ComponentWrapper<T_ACTUAL, Processor, T_JS_VALUE> final
    : public ComponentWrapperBase<T_ACTUAL, Processor, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, Processor, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  ProcessResult ProcessKeyEvent(const KeyEvent& keyEvent) override {
    return this->actual()->processKeyEvent(keyEvent, this->environment());
  }
};  // namespace rime
