#pragma once

#include <rime/config/config_types.h>
#include <filesystem>
#include <string>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

// Helper function to convert ConfigItem to JSValue recursively
// Uses auto return type to automatically handle QuickJS and JavaScriptCore
template <typename T_ENGINE>
static auto configItemToJsValue(T_ENGINE& engine, const an<ConfigItem> item)
    -> decltype(engine.null()) {
  if (!item) {
    return engine.null();
  }

  switch (item->type()) {
    case ConfigItem::kNull:
      return engine.null();

    case ConfigItem::kScalar: {
      const auto value = std::dynamic_pointer_cast<ConfigValue>(item);
      if (!value) {
        return engine.null();
      }
      bool boolVal = false;
      if (value->GetBool(&boolVal)) {
        return engine.wrap(boolVal);
      }
      int intVal = 0;
      if (value->GetInt(&intVal)) {
        return engine.wrap(intVal);
      }
      double doubleVal = 0.0;
      if (value->GetDouble(&doubleVal)) {
        return engine.wrap(doubleVal);
      }
      std::string strVal;
      if (value->GetString(&strVal)) {
        return engine.wrap(strVal.c_str());
      }
      return engine.null();
    }

    case ConfigItem::kList: {
      const auto list = std::dynamic_pointer_cast<ConfigList>(item);
      if (!list) {
        return engine.null();
      }
      auto jsArray = engine.newArray();
      for (size_t i = 0; i < list->size(); ++i) {
        auto listItem = list->GetAt(i);
        auto jsItem = configItemToJsValue(engine, listItem);
        engine.insertItemToArray(jsArray, i, jsItem);
      }
      return jsArray;
    }

    case ConfigItem::kMap: {
      const auto map = std::dynamic_pointer_cast<ConfigMap>(item);
      if (!map) {
        return engine.null();
      }
      auto jsObject = engine.newObject();
      for (const auto& [key, val] : *map) {
        auto jsItem = configItemToJsValue(engine, val);
        engine.setObjectProperty(jsObject, key.c_str(), jsItem);
      }
      return jsObject;
    }

    default:
      return engine.null();
  }
}

template <>
class JsWrapper<Config> {
  DEFINE_CFUNCTION_ARGC(loadFromFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    return engine.wrap(obj->LoadFromFile(std::filesystem::path(path.c_str())));
  })

  DEFINE_CFUNCTION_ARGC(saveToFile, 1, {
    std::string path = engine.toStdString(argv[0]);
    return engine.wrap(obj->SaveToFile(std::filesystem::path(path.c_str())));
  })

  DEFINE_CFUNCTION_ARGC(getBool, 1, {
    std::string key = engine.toStdString(argv[0]);
    bool value = false;
    bool success = obj->GetBool(key, &value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getInt, 1, {
    std::string key = engine.toStdString(argv[0]);
    int value = 0;
    bool success = obj->GetInt(key, &value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getDouble, 1, {
    std::string key = engine.toStdString(argv[0]);
    double value = 0.0;
    bool success = obj->GetDouble(key, &value);
    return success ? engine.wrap(value) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getString, 1, {
    std::string key = engine.toStdString(argv[0]);
    std::string value;
    bool success = obj->GetString(key, &value);
    return success ? engine.wrap(value.c_str()) : engine.null();
  })

  DEFINE_CFUNCTION_ARGC(getList, 1, {
    std::string key = engine.toStdString(argv[0]);
    auto list = obj->GetList(key);
    return engine.wrap(list);
  })

  DEFINE_CFUNCTION_ARGC(setBool, 2, {
    std::string key = engine.toStdString(argv[0]);
    bool value = engine.toBool(argv[1]);
    bool success = obj->SetBool(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION_ARGC(setInt, 2, {
    std::string key = engine.toStdString(argv[0]);
    int value = engine.toInt(argv[1]);
    bool success = obj->SetInt(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION_ARGC(setDouble, 2, {
    std::string key = engine.toStdString(argv[0]);
    double value = engine.toDouble(argv[1]);
    bool success = obj->SetDouble(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION_ARGC(setString, 2, {
    std::string key = engine.toStdString(argv[0]);
    std::string value = engine.toStdString(argv[1]);
    bool success = obj->SetString(key, value);
    return engine.wrap(success);
  })

  DEFINE_CFUNCTION(getObject, {
    // If no argument, return the entire config as a JS object
    if (argc == 0) {
      // Get the root map of the config
      auto item = obj->GetItem("");
      if (!item || item->type() != ConfigItem::kMap) {
        // Return empty object for non-map config
        return engine.newObject();
      }
      return configItemToJsValue(engine, item);
    }

    // If argument provided, get specific key
    std::string key = engine.toStdString(argv[0]);
    auto item = obj->GetItem(key);
    return configItemToJsValue(engine, item);
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Config,
                                WITH_CONSTRUCTOR(),
                                WITH_PROPERTIES(),
                                WITH_GETTERS(),
                                WITH_FUNCTIONS(loadFromFile,
                                               saveToFile,
                                               getBool,
                                               getInt,
                                               getDouble,
                                               getString,
                                               getList,
                                               getObject,
                                               setBool,
                                               setInt,
                                               setDouble,
                                               setString));
};
