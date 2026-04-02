#pragma once

#include <glog/logging.h>
#include <string>

#include "types/qjs_types.h"

template <typename T_JS_VALUE>
class QjsModule {
  using T_JS_OBJECT = typename JsEngine<T_JS_VALUE>::T_JS_OBJECT;

protected:
  QjsModule(const std::string& nameSpace, const Environment& environment, const char* mainFuncName);
  ~QjsModule();

  [[nodiscard]] bool isLoaded() const { return isLoaded_; }
  [[nodiscard]] T_JS_OBJECT getInstance() const { return instance_; }
  [[nodiscard]] T_JS_OBJECT getMainFunc() const { return mainFunc_; }
  [[nodiscard]] std::string getNamespace() const { return namespace_; }

private:
  const std::string namespace_;

  bool isLoaded_ = false;

  T_JS_OBJECT instance_;
  T_JS_OBJECT mainFunc_;
  T_JS_OBJECT finalizer_;

public:
  QjsModule(const QjsModule&) = delete;
  QjsModule(QjsModule&&) = delete;
  QjsModule& operator=(const QjsModule&) = delete;
  QjsModule& operator=(QjsModule&&) = delete;
};
