#pragma once

#include <glog/logging.h>
#include "dicts/trie.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_leveldb.h"

using namespace rime;

template <>
class JsWrapper<Trie> {
  DEFINE_CFUNCTION_ARGC(loadTextFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    ParseTextFileOptions options;
    if (argc > 1) {
      options = parseTextFileOptions(engine, argv[1]);
    }

    try {
      obj->loadTextFile(absolutePath, options);
    } catch (const std::exception& e) {
      LOG(ERROR) << "loadTextFile of " << absolutePath << " failed: " << e.what();
      return engine.throwError(JsErrorType::GENERIC, e.what());
    }

    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(loadBinaryFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    try {
      obj->loadBinaryFile(absolutePath);
    } catch (const std::exception& e) {
      LOG(ERROR) << "loadBinaryFileMmap of " << absolutePath << " failed: " << e.what();
      return engine.throwError(JsErrorType::GENERIC, e.what());
    }
    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(saveToBinaryFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    try {
      obj->saveToBinaryFile(absolutePath);
    } catch (const std::exception& e) {
      LOG(ERROR) << "saveToBinaryFile of " << absolutePath << " failed: " << e.what();
      return engine.throwError(JsErrorType::GENERIC, e.what());
    }
    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(find, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto result = obj->find(key);
    return result.has_value() ? engine.wrap(result.value()) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(prefixSearch, 1, {
    std::string prefix = engine.toStdString(argv[0]);
    auto matches = obj->prefixSearch(prefix);

    auto jsArray = engine.newArray();
    for (size_t i = 0; i < matches.size(); ++i) {
      auto jsObject = engine.newObject();
      engine.setObjectProperty(jsObject, "text", engine.wrap(matches[i].first));
      engine.setObjectProperty(jsObject, "info", engine.wrap(matches[i].second));
      engine.insertItemToArray(jsArray, i, jsObject);
    }
    return jsArray;
  })
  DEFINE_CFUNCTION(makeTrie, { return engine.wrap(std::make_shared<Trie>()); })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      Trie,
      WITH_CONSTRUCTOR(makeTrie),
      WITH_PROPERTIES(),
      WITH_GETTERS(),
      WITH_FUNCTIONS(loadTextFile, loadBinaryFile, saveToBinaryFile, find, prefixSearch));
};
