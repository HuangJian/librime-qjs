#include "qjs_translation.h"
#include <glog/logging.h>

template <typename T_JS_VALUE>
QuickJSTranslation<T_JS_VALUE>::QuickJSTranslation(an<Translation> translation,
                                                   const T_JS_VALUE& filterObj,
                                                   const T_JS_VALUE& filterFunc,
                                                   Environment* environment)
    : PrefetchTranslation(std::move(translation)), replenished_(true) {
  doFilter(filterObj, filterFunc, environment);
  set_exhausted(cache_.empty());
}

template <typename T_JS_VALUE>
bool QuickJSTranslation<T_JS_VALUE>::doFilter(const T_JS_VALUE& filterObj,
                                              const T_JS_VALUE& filterFunc,
                                              Environment* environment) {
  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  auto jsArray = jsEngine.newArray();
  size_t idx = 0;
  while (auto candidate = translation_->exhausted() ? nullptr : translation_->Peek()) {
    translation_->Next();
    jsEngine.insertItemToArray(jsArray, idx++, jsEngine.wrap(candidate));
  }
  if (idx == 0) {
    jsEngine.freeValue(jsArray);
    return true;
  }

  auto jsEnvironment = jsEngine.wrap(environment);
  T_JS_VALUE args[2];
  args[0] = std::move(jsArray);
  args[1] = std::move(jsEnvironment);
  T_JS_VALUE resultArray = jsEngine.callFunction(filterFunc, filterObj, 2, args);

  if (!jsEngine.isArray(resultArray)) {
    LOG(ERROR) << "[qjs] Failed to filter candidates";
    return false;
  }

  const size_t length = jsEngine.getArrayLength(resultArray);
  for (size_t i = 0; i < length; i++) {
    T_JS_VALUE item = jsEngine.getArrayItem(resultArray, i);
    if (auto candidate = jsEngine.template unwrap<Candidate>(item)) {
      cache_.push_back(candidate);
    } else {
      LOG(ERROR) << "[qjs] Failed to unwrap candidate at index " << i;
    }
  }

  return true;
}

template <typename T_JS_VALUE>
QuickJSFastTranslation<T_JS_VALUE>::QuickJSFastTranslation(const an<Translation>& translation,
                                                           const T_JS_OBJECT& filterObj,
                                                           const T_JS_OBJECT& filterFunc,
                                                           Environment* environment) {
  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  auto iterator = jsEngine.wrap(translation);
  auto jsEnv = jsEngine.wrap(environment);
  T_JS_VALUE args[2];
  args[0] = std::move(iterator);
  args[1] = std::move(jsEnv);
  generator_ = jsEngine.callFunction(filterFunc, filterObj, 2, args);
  nextFunction_ = jsEngine.getObjectProperty(generator_, "next");
}

template <typename T_JS_VALUE>
QuickJSFastTranslation<T_JS_VALUE>::~QuickJSFastTranslation() = default;

template <typename T_JS_VALUE>
bool QuickJSFastTranslation<T_JS_VALUE>::Next() {
  if (this->exhausted()) {
    return false;
  }

  invokeGenerator();

  if (upstream_ != nullptr) {
    // `return iter;` was called in js side, return the upstream data
    const auto ret = upstream_->Next();
    this->set_exhausted(upstream_->exhausted());
    return ret;
  }

  return !this->exhausted();
}

template <typename T_JS_VALUE>
an<Candidate> QuickJSFastTranslation<T_JS_VALUE>::Peek() {
  if (!isGeneratorEverInvoked_) {
    invokeGenerator();
  }
  if (this->exhausted()) {
    return nullptr;
  }
  if (upstream_ != nullptr) {
    // `return iter;` was called in js side, return the upstream data
    return upstream_->exhausted() ? nullptr : upstream_->Peek();
  }

  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  auto jsValue = jsEngine.getObjectProperty(nextResult_, "value");
  auto ret = jsEngine.template unwrap<Candidate>(jsValue);
  return ret;
}

template <typename T_JS_VALUE>
void QuickJSFastTranslation<T_JS_VALUE>::invokeGenerator() {
  if (upstream_ != nullptr) {
    return;
  }

  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();

  nextResult_ = jsEngine.callFunction(nextFunction_, generator_, 0, nullptr);
  isGeneratorEverInvoked_ = true;
  if (jsEngine.isException(nextResult_)) {
    LOG(ERROR) << "[qjs] Exception thrown while filtering candidates with iterator";
    set_exhausted(true);
    return;
  }

  auto jsDone = jsEngine.getObjectProperty(nextResult_, "done");
  const bool isDone = jsEngine.isBool(jsDone) && jsEngine.toBool(jsDone);
  if (isDone) {
    // check the return value of the generator,
    auto jsValue = jsEngine.getObjectProperty(nextResult_, "value");
    if (auto upstream = jsEngine.template unwrap<Translation>(jsValue)) {
      // it returned the upstream translation with `return iter;` in js side
      upstream_ = upstream;
      set_exhausted(upstream->exhausted());
    } else {
      // it returned undefined with `return;` in js side
      set_exhausted(true);
    }
  }
}

template class QuickJSTranslation<QjsValueRAII>;
template class QuickJSFastTranslation<QjsValueRAII>;

#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSTranslation<JSValueRef>;
template class QuickJSFastTranslation<JSValueRef>;
#endif
