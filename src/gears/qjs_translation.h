#pragma once

#include <rime/translation.h>

#include "engines/common.h"
#include "environment.h"

using namespace rime;

template <typename T_JS_VALUE>
class QuickJSTranslation : public rime::PrefetchTranslation {
public:
  QuickJSTranslation(const QuickJSTranslation&) = delete;
  QuickJSTranslation(QuickJSTranslation&&) = delete;
  QuickJSTranslation& operator=(const QuickJSTranslation&) = delete;
  QuickJSTranslation& operator=(QuickJSTranslation&&) = delete;

  QuickJSTranslation(rime::an<rime::Translation> translation,
                     const T_JS_VALUE& filterObj,
                     const T_JS_VALUE& filterFunc,
                     Environment* environment);
  ~QuickJSTranslation() override = default;

protected:
  bool Replenish() override { return replenished_; }

private:
  bool doFilter(const T_JS_VALUE& filterObj,
                const T_JS_VALUE& filterFunc,
                Environment* environment);

  bool replenished_ = false;
};

template <typename T_JS_VALUE>
class QuickJSFastTranslation : public rime::Translation {
  using T_JS_OBJECT = typename JsEngine<T_JS_VALUE>::T_JS_OBJECT;

  bool isGeneratorEverInvoked_ = false;
  T_JS_OBJECT generator_;
  T_JS_OBJECT nextFunction_;
  T_JS_OBJECT nextResult_;

  an<Translation> upstream_{nullptr};

public:
  QuickJSFastTranslation(const QuickJSFastTranslation&) = delete;
  QuickJSFastTranslation(QuickJSFastTranslation&&) = delete;
  QuickJSFastTranslation& operator=(const QuickJSFastTranslation&) = delete;
  QuickJSFastTranslation& operator=(QuickJSFastTranslation&&) = delete;

  QuickJSFastTranslation(const rime::an<rime::Translation>& translation,
                         const T_JS_OBJECT& filterObj,
                         const T_JS_OBJECT& filterFunc,
                         Environment* environment);

  ~QuickJSFastTranslation() override;

  bool Next() override;
  an<rime::Candidate> Peek() override;

private:
  void invokeGenerator();
};
