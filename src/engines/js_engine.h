#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "engines/js_exception.h"
#include "engines/type_map.h"

template <typename T_RIME_TYPE, typename T_JS_VALUE>
class JsWrapper;

template <typename T_JS_VALUE>
class JsEngine {
  using T_JS_OBJECT = typename TypeMap<T_JS_VALUE>::ObjectType;

public:
  void setBaseFolderPath(const char* absolutePath);

  int64_t getMemoryUsage();

  T_JS_VALUE loadJsFile(const char* fileName);
  T_JS_VALUE getJsClassHavingMethod(const T_JS_VALUE& container, const char* methodName);
  T_JS_VALUE getMethodOfClassOrInstance(T_JS_VALUE jsClass,
                                        T_JS_VALUE instance,
                                        const char* methodName);

  template <typename T>
  void* getOpaque(T_JS_OBJECT value);
  void setOpaque(T_JS_OBJECT value, void* opaque);

  T_JS_VALUE null();
  T_JS_VALUE undefined();
  T_JS_VALUE jsTrue();
  T_JS_VALUE jsFalse();

  T_JS_VALUE newArray();
  size_t getArrayLength(const T_JS_VALUE& array);
  int insertItemToArray(T_JS_VALUE array, size_t index, const T_JS_VALUE& value);
  T_JS_VALUE getArrayItem(const T_JS_VALUE& array, size_t index);

  T_JS_OBJECT newObject();
  T_JS_VALUE getObjectProperty(const T_JS_OBJECT& obj, const char* propertyName);
  int setObjectProperty(const T_JS_OBJECT& obj, const char* propertyName, const T_JS_VALUE& value);

  using ExposeFunction = T_JS_VALUE (*)(typename TypeMap<T_JS_VALUE>::ContextType ctx,
                                        T_JS_VALUE thisVal,
                                        int argc,
                                        T_JS_VALUE* argv);
  int setObjectFunction(T_JS_OBJECT obj,
                        const char* functionName,
                        ExposeFunction cppFunction,
                        int expectingArgc);

  T_JS_OBJECT toObject(const T_JS_VALUE& value);
  T_JS_VALUE toJsString(const char* str);
  T_JS_VALUE toJsString(const std::string& str);
  std::string toStdString(const T_JS_VALUE& value);

  T_JS_VALUE toJsBool(bool value);
  bool toBool(T_JS_VALUE value);

  T_JS_VALUE toJsInt(size_t value);
  size_t toInt(const T_JS_VALUE& value);

  T_JS_VALUE toJsDouble(bool value);
  double toDouble(const T_JS_VALUE& value);

  T_JS_VALUE callFunction(T_JS_VALUE func, T_JS_VALUE thisArg, int argc, T_JS_VALUE* argv);
  T_JS_VALUE newClassInstance(T_JS_VALUE clazz, int argc, T_JS_VALUE* argv);

  bool isObject(const T_JS_VALUE& value);
  bool isNull(const T_JS_VALUE& value);
  bool isUndefined(const T_JS_VALUE& value);
  bool isException(const T_JS_VALUE& value);
  T_JS_OBJECT throwError(JsErrorType errorType, const char* format, ...);

  void logErrorStackTrace(const T_JS_OBJECT& exception,
                          const char* file = __FILE_NAME__,
                          int line = __LINE__);

  void freeValue(const T_JS_VALUE& value);

  template <typename T_RIME_TYPE>
  void registerType(JsWrapper<T_RIME_TYPE, T_JS_VALUE>& wrapper);

  typename TypeMap<T_JS_VALUE>::ExposeFunctionType defineFunction(const char* name,
                                                                  int avgc,
                                                                  ExposeFunction func);

  typename TypeMap<T_JS_VALUE>::ExposePropertyType defineProperty(
      const char* name,
      typename TypeMap<T_JS_VALUE>::GetterFunctionType getter,
      typename TypeMap<T_JS_VALUE>::SetterFunctionType setter);

  template <typename T>
  T_JS_OBJECT wrap(T* ptrValue);
  template <typename T>
  T* unwrap(const T_JS_VALUE& value);

  template <typename T>
  T_JS_OBJECT wrapShared(const std::shared_ptr<T>& value);
  template <typename T>
  std::shared_ptr<T> unwrapShared(const T_JS_VALUE& value);

  virtual T_JS_VALUE eval(const char* code, const char* filename = "<eval>");

  virtual T_JS_VALUE getGlobalObject();
};
