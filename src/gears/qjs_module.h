#pragma once

#include <glog/logging.h>
#include <string>
#include <vector>

#include "types/qjs_types.h"

template <typename T_JS_VALUE>
class QjsModule {
protected:
  QjsModule(const std::string& nameSpace, Environment* environment, const char* mainFuncName)
      : namespace_(nameSpace) {
    // the js engine is lazy loaded, so we need to register the types first
    registerTypesToJsEngine<T_JS_VALUE>();
    std::filesystem::path path(rime_get_api()->get_user_data_dir());
    path.append("js");
    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    jsEngine.setBaseFolderPath(path.generic_string().c_str());

    auto jsEnvironment = jsEngine.wrap(environment);
    std::vector<T_JS_VALUE> args = {jsEnvironment};
    instance_ = jsEngine.createInstanceOfModule(namespace_.c_str(), args, mainFuncName);
    jsEngine.freeValue(jsEnvironment);

    if (!jsEngine.isObject(instance_)) {
      jsEngine.freeValue(instance_);
      LOG(ERROR) << "[qjs] Error creating an instance of the exported class in " << nameSpace;
      return;
    }

    mainFunc_ = jsEngine.toObject(jsEngine.getObjectProperty(instance_, mainFuncName));
    finalizer_ = jsEngine.toObject(jsEngine.getObjectProperty(instance_, "finalizer"));

    jsEngine.protectFromGC(instance_, mainFunc_, finalizer_);

    isLoaded_ = true;
    LOG(INFO) << "[qjs] created an instance of the exported class in " << nameSpace;
  }

  ~QjsModule() {
    if (!isLoaded_) {
      return;
    }

    auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
    if (jsEngine.isUndefined(finalizer_)) {
      LOG(INFO) << "[qjs] ~" << namespace_ << " no `finalizer` function exported.";
    } else {
      LOG(INFO) << "[qjs] running the finalizer function of " << namespace_;
      T_JS_VALUE finalizerResult = jsEngine.callFunction(finalizer_, instance_, 0, nullptr);
      if (jsEngine.isException(finalizerResult)) {
        LOG(ERROR) << "[qjs] ~" << namespace_ << " Error running the finalizer function.";
      }
      jsEngine.freeValue(finalizerResult);
    }

    jsEngine.unprotectFromGC(instance_, mainFunc_, finalizer_);
    jsEngine.freeValue(instance_, mainFunc_, finalizer_);
  }

  [[nodiscard]] bool isLoaded() const { return isLoaded_; }
  [[nodiscard]] typename JsEngine<T_JS_VALUE>::T_JS_OBJECT getInstance() const { return instance_; }
  [[nodiscard]] typename JsEngine<T_JS_VALUE>::T_JS_OBJECT getMainFunc() const { return mainFunc_; }
  [[nodiscard]] std::string getNamespace() const { return namespace_; }

private:
  const std::string namespace_;

  bool isLoaded_ = false;

  typename JsEngine<T_JS_VALUE>::T_JS_OBJECT instance_;
  typename JsEngine<T_JS_VALUE>::T_JS_OBJECT mainFunc_;
  typename JsEngine<T_JS_VALUE>::T_JS_OBJECT finalizer_;

public:
  QjsModule(const QjsModule&) = delete;
  QjsModule(QjsModule&&) = delete;
  QjsModule& operator=(const QjsModule&) = delete;
  QjsModule& operator=(QjsModule&&) = delete;
};
