#pragma once

#include <rime/gear/translator_commons.h>
#include <rime/translator.h>

#include <rime/translation.h>

#include "qjs_component.h"
#include "qjs_module.h"

using namespace rime;

template <typename T_JS_VALUE>
class QuickJSTranslator : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSTranslator(const Ticket& ticket, const Environment* environment);

  an<Translation> query(const std::string& input,
                        const Segment& segment,
                        const Environment* environment);
};

// Specialization for Translator
template <typename T_ACTUAL, typename T_JS_VALUE>
class rime::ComponentWrapper<T_ACTUAL, Translator, T_JS_VALUE> final
    : public ComponentWrapperBase<T_ACTUAL, Translator, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, Translator, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual an<Translation> Query(const std::string& input, const Segment& segment) override {
    return this->actual()->query(input, segment, this->environment());
  }
};  // namespace rime
