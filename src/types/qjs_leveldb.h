#pragma once

#include <glog/logging.h>

#include "dicts/leveldb.h"
#include "engines/js_macros.h"
#include "js_wrapper.h"

template <typename T>
static ParseTextFileOptions parseTextFileOptions(const JsEngine<T>& engine, T jsOptions) {
  ParseTextFileOptions result;
  if (engine.isUndefined(jsOptions)) {
    return result;
  }

  auto objOptions = engine.toObject(jsOptions);

  auto jsIsReversed = engine.getObjectProperty(objOptions, "isReversed");
  if (engine.isBool(jsIsReversed)) {
    result.isReversed = engine.toBool(jsIsReversed);
  }
  auto jsCharsToRemove = engine.getObjectProperty(objOptions, "charsToRemove");
  if (!engine.isUndefined(jsCharsToRemove)) {
    result.charsToRemove = engine.toStdString(jsCharsToRemove);
  }
  auto jsLines = engine.getObjectProperty(objOptions, "lines");
  if (!engine.isUndefined(jsLines)) {
    result.lines = engine.toInt(jsLines);
  }
  auto jsDelimiter = engine.getObjectProperty(objOptions, "delimiter");
  if (!engine.isUndefined(jsDelimiter)) {
    result.delimiter = engine.toStdString(jsDelimiter);
  }
  auto jsComment = engine.getObjectProperty(objOptions, "comment");
  if (!engine.isUndefined(jsComment)) {
    result.comment = engine.toStdString(jsComment);
  }
  auto jsOnDuplicatedKey = engine.getObjectProperty(objOptions, "onDuplicatedKey");
  if (!engine.isUndefined(jsOnDuplicatedKey)) {
    auto onDuplicatedKey = engine.toStdString(jsOnDuplicatedKey);
    if (onDuplicatedKey == "Skip") {
      result.onDuplicatedKey = OnDuplicatedKey::Skip;
    } else if (onDuplicatedKey == "Concat") {
      result.onDuplicatedKey = OnDuplicatedKey::Concat;
    } else {  // default to overwrite
      result.onDuplicatedKey = OnDuplicatedKey::Overwrite;
    }
  }
  auto jsConcatSeparator = engine.getObjectProperty(objOptions, "concatSeparator");
  if (!engine.isUndefined(jsConcatSeparator)) {
    result.concatSeparator = engine.toStdString(jsConcatSeparator);
  }
  return result;
}

template <>
class JsWrapper<LevelDb> {
  DEFINE_CFUNCTION_ARGC(loadTextFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    ParseTextFileOptions options;
    if (argc > 1) {
      options = parseTextFileOptions(engine, argv[1]);
    }

    obj->loadTextFile(absolutePath, options);
    return engine.undefined();
  })

  DEFINE_CFUNCTION_ARGC(loadBinaryFile, 1, {
    std::string absolutePath = engine.toStdString(argv[0]);
    obj->loadBinaryFile(absolutePath);
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
    return result ? engine.wrap(*result) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(prefixSearch, 1, {
    std::string prefix = engine.toStdString(argv[0]);
    auto results = obj->prefixSearch(prefix);

    auto jsArray = engine.newArray();
    for (size_t i = 0; i < results.size(); ++i) {
      auto jsObject = engine.newObject();
      engine.setObjectProperty(jsObject, "text", engine.wrap(results[i].first));
      engine.setObjectProperty(jsObject, "info", engine.wrap(results[i].second));
      engine.insertItemToArray(jsArray, i, jsObject);
    }
    return jsArray;
  })

  DEFINE_CFUNCTION(close, {
    obj->close();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(makeLevelDb, { return engine.wrap(std::make_shared<LevelDb>()); })

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      LevelDb,
      WITH_CONSTRUCTOR(makeLevelDb),
      WITHOUT_PROPERTIES,
      WITHOUT_GETTERS,
      WITH_FUNCTIONS(loadTextFile, loadBinaryFile, saveToBinaryFile, find, prefixSearch, close));
};
