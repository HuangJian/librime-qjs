#pragma once

#include <rime/filter.h>
#include <chrono>
#include <memory>

#include "environment.h"
#include "qjs_component.h"
#include "qjs_module.h"
#include "qjs_translation.h"

template <typename T_JS_VALUE>
class QuickJSFilter : public QjsModule<T_JS_VALUE> {
  inline static std::chrono::time_point<std::chrono::steady_clock> beginClock =
      std::chrono::steady_clock::now();

  typename JsEngine<T_JS_VALUE>::T_JS_OBJECT funcIsApplicable_;
  bool isFilterFuncGenerator_ = false;

public:
  QuickJSFilter(const QuickJSFilter&) = delete;
  QuickJSFilter(QuickJSFilter&&) = delete;
  QuickJSFilter& operator=(const QuickJSFilter&) = delete;
  QuickJSFilter& operator=(QuickJSFilter&&) = delete;

  explicit QuickJSFilter(const Ticket& ticket, Environment* environment);

  ~QuickJSFilter();

  [[nodiscard]] bool isFilterFuncGenerator() const;

  std::shared_ptr<Translation> apply(std::shared_ptr<Translation> translation,
                                     Environment* environment);
};

// Specialization for Filter
template <typename T_ACTUAL, typename T_JS_VALUE>
class rime::ComponentWrapper<T_ACTUAL, Filter, T_JS_VALUE> final
    : public ComponentWrapperBase<T_ACTUAL, Filter, T_JS_VALUE> {
public:
  explicit ComponentWrapper(const Ticket& ticket)
      : ComponentWrapperBase<T_ACTUAL, Filter, T_JS_VALUE>(ticket) {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  virtual an<Translation> Apply(an<Translation> translation, CandidateList* candidates) override {
    return this->actual()->apply(translation, this->environment());
  }
};  // namespace rime
