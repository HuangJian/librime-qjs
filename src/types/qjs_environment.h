#pragma once

#include <rime/engine.h>

#include "engines/js_exception.h"
#include "engines/js_macros.h"
#include "environment.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Environment> {
  DEFINE_GETTER(Environment, id, obj->getId())
  DEFINE_GETTER(Environment, engine, obj->getEngine())
  DEFINE_GETTER(Environment, namespace, obj->getNameSpace())
  DEFINE_GETTER(Environment, os, obj->getSystemInfo())
  DEFINE_GETTER(Environment, userDataDir, obj->getUserDataDir())
  DEFINE_GETTER(Environment, sharedDataDir, obj->getSharedDataDir())

  DEFINE_CFUNCTION_ARGC(loadFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }
    return engine.wrap(Environment::loadFile(path));
  })

  DEFINE_CFUNCTION_ARGC(fileExists, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }
    return engine.wrap(Environment::fileExists(path));
  })

  DEFINE_CFUNCTION_ARGC(saveFile, 2, {
    std::string path = engine.toStdString(argv[0]);
    std::string content = engine.toStdString(argv[1]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }
    return engine.wrap(Environment::saveFile(path, content));
  })

  DEFINE_CFUNCTION_ARGC(removeFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The absolutePath argument should be a string");
    }
    return engine.wrap(Environment::removeFile(path));
  })

  DEFINE_CFUNCTION_ARGC(createDir, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The path argument should be a string");
    }
    bool exist_ok = false;
    if (argc > 1) {
      // Check if second argument is a boolean
      if (!engine.isBool(argv[1])) {
        throw JsException(JsErrorType::TYPE, "The exist_ok argument must be a boolean");
      }
      exist_ok = engine.toBool(argv[1]);
    }
    return engine.wrap(Environment::createDir(path, exist_ok));
  })

  DEFINE_CFUNCTION_ARGC(removeDir, 1, {
    std::string path = engine.toStdString(argv[0]);
    if (path.empty()) {
      throw JsException(JsErrorType::SYNTAX, "The path argument should be a string");
    }
    return engine.wrap(Environment::removeDir(path));
  })

  DEFINE_CFUNCTION(getRimeInfo, {
    auto info = Environment::getRimeInfo();
    int64_t bytes = engine.getMemoryUsage();
    if (bytes >= 0) {
      info = info + " | " + engine.engineName + " Mem: " + Environment::formatMemoryUsage(bytes);
    }
    return engine.wrap(info);
  })

  DEFINE_CFUNCTION_ARGC(popen, 1, {
    std::string command = engine.toStdString(argv[0]);
    if (command.empty()) {
      return engine.throwError(JsErrorType::SYNTAX, "The command argument should be a string");
    }
    auto timeout = argc > 1 ? engine.toInt(argv[1]) : 1000;  // default to 1000ms
    auto result = Environment::popen(command, timeout);
    if (!result.second.empty()) {
      return engine.throwError(JsErrorType::GENERIC, result.second);
    }
    return engine.wrap(result.first);
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Environment,
                                WITHOUT_CONSTRUCTOR,
                                WITHOUT_PROPERTIES,
                                WITH_GETTERS(id, engine, namespace, userDataDir, sharedDataDir, os),
                                WITH_FUNCTIONS(loadFile,
                                               1,
                                               fileExists,
                                               1,
                                               saveFile,
                                               2,
                                               removeFile,
                                               1,
                                               createDir,
                                               1,
                                               removeDir,
                                               1,
                                               getRimeInfo,
                                               0,
                                               popen,
                                               1));
};
