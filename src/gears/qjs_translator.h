#pragma once

#include <rime/gear/translator_commons.h>
#include <rime/translator.h>

#include <rime/composition.h>
#include <rime/context.h>
#include <rime/translation.h>

#include "engines/common.h"
#include "qjs_component.h"
#include "qjs_module.h"

using namespace rime;

template <typename T_JS_VALUE>
class QuickJSTranslator : public QjsModule<T_JS_VALUE> {
public:
  explicit QuickJSTranslator(const rime::Ticket& ticket, Environment* environment);

  rime::an<rime::Translation> query(const std::string& input,
                                    const rime::Segment& segment,
                                    Environment* environment);
};

// Specialization for Translator
namespace rime {
template <typename T_ACTUAL, typename T_JS_VALUE>
class ComponentWrapper<T_ACTUAL, rime::Translator, T_JS_VALUE>
    : public ComponentWrapperBase<T_ACTUAL, rime::Translator, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const rime::Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, rime::Translator, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual rime::an<rime::Translation> Query(const std::string& input, const rime::Segment& segment);
};
}  // namespace rime
