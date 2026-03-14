#include "qjs_filter.h"
#include <type_traits>

template <typename T_JS_VALUE>
QuickJSFilter<T_JS_VALUE>::QuickJSFilter(const Ticket& ticket, Environment* environment)
    : QjsModule<T_JS_VALUE>(ticket.name_space, environment, "filter") {
  if (!this->isLoaded()) {
    return;
  }
  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  funcIsApplicable_ =
      jsEngine.toObject(jsEngine.getObjectProperty(this->getInstance(), "isApplicable"));
  jsEngine.protectFromGC(funcIsApplicable_);

  isFilterFuncGenerator_ = isFilterFuncGenerator();
}

template <typename T_JS_VALUE>
QuickJSFilter<T_JS_VALUE>::~QuickJSFilter() {
  if (!this->isLoaded()) {
    return;
  }

  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  if (jsEngine.isFunction(funcIsApplicable_)) {
    jsEngine.unprotectFromGC(funcIsApplicable_);
    jsEngine.freeValue(funcIsApplicable_);
  }
}

template <typename T_JS_VALUE>
bool QuickJSFilter<T_JS_VALUE>::isFilterFuncGenerator() const {
  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  const auto& filterFunc = this->getMainFunc();
  if (jsEngine.isFunction(filterFunc)) {
    auto proto = jsEngine.getObjectProperty(filterFunc, "constructor");
    if (jsEngine.isObject(proto)) {
      auto jsName = jsEngine.getObjectProperty(jsEngine.toObject(proto), "name");
      auto name = jsEngine.toStdString(jsName);
      jsEngine.freeValue(jsName, proto);
      return name == "GeneratorFunction";
    }

    jsEngine.freeValue(proto);
  }
  return false;
}

template <typename T_JS_VALUE>
std::shared_ptr<Translation> QuickJSFilter<T_JS_VALUE>::apply(
    std::shared_ptr<Translation> translation,
    Environment* environment) {
  if (this->getNamespace().find("benchmark_begin") != std::string::npos) {
    beginClock = std::chrono::steady_clock::now();
  } else if (this->getNamespace().find("benchmark_end") != std::string::npos) {
    const auto endClock = std::chrono::steady_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(endClock - beginClock);
    std::string engine = "jsc";
    if constexpr (std::is_same_v<T_JS_VALUE, QjsValueRAII>) {
      engine = "qjs";
    }
    constexpr int PADDING = 6;
    LOG(INFO) << "[benchmark] all " << engine << " filters run for " << std::setw(PADDING)
              << duration.count()
              << " us, with input = " << environment->getEngine()->context()->input();
  }

  if (!this->isLoaded()) {
    return translation;
  }

  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  if (jsEngine.isFunction(funcIsApplicable_)) {
    auto jsEvn = jsEngine.wrap(environment);
    T_JS_VALUE args[1] = {jsEvn};
    auto result = jsEngine.callFunction(funcIsApplicable_, this->getInstance(), 1, args);
    const bool isApplicable = jsEngine.isBool(result) && jsEngine.toBool(result);
    jsEngine.freeValue(jsEvn, result);
    if (!isApplicable) {
      return translation;
    }
  }

  if (isFilterFuncGenerator_) {
    return std::make_shared<QuickJSFastTranslation<T_JS_VALUE>>(translation, this->getInstance(),
                                                                this->getMainFunc(), environment);
  }

  return std::make_shared<QuickJSTranslation<T_JS_VALUE>>(translation, this->getInstance(),
                                                          this->getMainFunc(), environment);
}

namespace rime {

template class ComponentWrapper<QuickJSFilter<QjsValueRAII>, Filter, QjsValueRAII>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class ComponentWrapper<QuickJSFilter<JSValueRef>, Filter, JSValueRef>;
#endif
}  // namespace rime

template class QuickJSFilter<QjsValueRAII>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class QuickJSFilter<JSValueRef>;
#endif
