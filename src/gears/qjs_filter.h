#pragma once

#include <rime/context.h>
#include <rime/engine.h>
#include <rime/filter.h>
#include <rime/gear/filter_commons.h>
#include <chrono>
#include <iomanip>
#include <memory>

#include "environment.h"
#include "qjs_component.h"
#include "qjs_module.h"
#include "qjs_translation.h"

template <typename T_JS_VALUE>
class QuickJSFilter : public QjsModule<T_JS_VALUE> {
  inline static std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>
      beginClock = std::chrono::steady_clock::now();

  typename JsEngine<T_JS_VALUE>::T_JS_OBJECT funcIsApplicable_;
  bool isFilterFuncGenerator_ = false;

public:
  QuickJSFilter(const QuickJSFilter&) = delete;
  QuickJSFilter(QuickJSFilter&&) = delete;
  QuickJSFilter& operator=(const QuickJSFilter&) = delete;
  QuickJSFilter& operator=(QuickJSFilter&&) = delete;

  explicit QuickJSFilter(const rime::Ticket& ticket, Environment* environment);

  ~QuickJSFilter();

  [[nodiscard]] bool isFilterFuncGenerator() const;

  std::shared_ptr<rime::Translation> apply(std::shared_ptr<rime::Translation> translation,
                                           Environment* environment);
};

// Specialization for Filter
namespace rime {
template <typename T_ACTUAL, typename T_JS_VALUE>
class ComponentWrapper<T_ACTUAL, rime::Filter, T_JS_VALUE>
    : public ComponentWrapperBase<T_ACTUAL, rime::Filter, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const rime::Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, rime::Filter, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual rime::an<rime::Translation> Apply(rime::an<rime::Translation> translation,
                                            rime::CandidateList* candidates) override {
    return this->actual()->apply(translation, this->environment());
  }
};
}  // namespace rime
