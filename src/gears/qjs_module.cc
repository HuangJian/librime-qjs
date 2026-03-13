#include "qjs_module.h"
#include <filesystem>
#include <rime_api.h>

template <typename T_JS_VALUE>
QjsModule<T_JS_VALUE>::QjsModule(const std::string& nameSpace,
                                 Environment* environment,
                                 const char* mainFuncName)
    : namespace_(nameSpace) {
  // the js engine is lazy loaded, so we need to register the types first
  registerTypesToJsEngine<T_JS_VALUE>();

  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();

  const char* dataDirArray[2] = {rime_get_api()->get_user_data_dir(),
                                 rime_get_api()->get_shared_data_dir()};
  for (const auto* dataDir : dataDirArray) {
    if (dataDir == nullptr) continue;
    std::filesystem::path path(dataDir);
    path.append("js");
    jsEngine.setBaseFolderPath(path.generic_string().c_str());
  }

  auto jsEnvironment = jsEngine.wrap(environment);
  std::vector<T_JS_VALUE> args = {jsEnvironment};
  instance_ = jsEngine.createInstanceOfModule(namespace_.c_str(), args, mainFuncName);
  jsEngine.freeValue(jsEnvironment);

  if (jsEngine.isException(instance_) || !jsEngine.isObject(instance_)) {
    jsEngine.freeValue(instance_);
    LOG(ERROR) << "[qjs] Error creating an instance of the exported class in " << nameSpace;

    // set the fields to "undefined" to avoid crashing in destruction when calling `jsEngine.freeValue(instance_, mainFunc_, finalizer_)`
    instance_ = jsEngine.toObject(jsEngine.undefined());
    mainFunc_ = jsEngine.toObject(jsEngine.undefined());
    finalizer_ = jsEngine.toObject(jsEngine.undefined());
    return;
  }

  mainFunc_ = jsEngine.toObject(jsEngine.getObjectProperty(instance_, mainFuncName));
  finalizer_ = jsEngine.toObject(jsEngine.getObjectProperty(instance_, "finalizer"));

  jsEngine.protectFromGC(instance_, mainFunc_, finalizer_);

  isLoaded_ = true;
  LOG(INFO) << "[qjs] created an instance of the exported class in " << nameSpace;
}

template <typename T_JS_VALUE>
QjsModule<T_JS_VALUE>::~QjsModule() {
  auto& jsEngine = JsEngine<T_JS_VALUE>::instance();
  if (jsEngine.isUndefined(finalizer_)) {
    DLOG(INFO) << "[qjs] ~" << namespace_ << " no `finalizer` function exported.";
  } else if (isLoaded_) {
    DLOG(INFO) << "[qjs] running the finalizer function of " << namespace_;
    T_JS_VALUE finalizerResult = jsEngine.callFunction(finalizer_, instance_, 0, nullptr);
    if (jsEngine.isException(finalizerResult)) {
      LOG(ERROR) << "[qjs] ~" << namespace_ << " Error running the finalizer function.";
    }
    jsEngine.freeValue(finalizerResult);
  }

  if (isLoaded_) {
    jsEngine.unprotectFromGC(instance_, mainFunc_, finalizer_);
  }
  jsEngine.freeValue(instance_, mainFunc_, finalizer_);
}

template class QjsModule<JSValue>;
#ifdef _ENABLE_JAVASCRIPTCORE
template class QjsModule<JSValueRef>;
#endif
