#pragma once

#include <rime/engine.h>
#include <rime/schema.h>
#include <rime_api.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>
#include <utility>
#include "misc/system_info.h"

class Environment {
public:
  explicit Environment(rime::Engine* engine, std::string nameSpace)
      : id_(generateUuid()), engine_(engine), nameSpace_(std::move(nameSpace)) {}

  [[nodiscard]] std::string getId() const { return id_; }
  [[nodiscard]] rime::Engine* getEngine() const { return engine_; }
  [[nodiscard]] const std::string& getNameSpace() const { return nameSpace_; }
  [[nodiscard]] SystemInfo* getSystemInfo() { return &systemInfo_; }
  static std::string getUserDataDir() { return rime_get_api()->get_user_data_dir(); }
  static std::string getSharedDataDir() { return rime_get_api()->get_shared_data_dir(); }

  static std::string loadFile(const std::string& path);
  static bool fileExists(const std::string& path);
  static bool saveFile(const std::string& path, const std::string& content);
  static bool removeFile(const std::string& path);
  static bool createDir(const std::string& path, bool existOk = false);
  static bool removeDir(const std::string& path);
  static std::string getRimeInfo();
  static std::pair<std::string, std::string> popen(const std::string& command,
                                                   int timeoutInMilliseconds);

  static std::string formatMemoryUsage(size_t usage);

private:
  std::string id_;
  rime::Engine* engine_;
  std::string nameSpace_;
  SystemInfo systemInfo_{};

  static std::string generateUuid() {
    boost::uuids::random_generator gen;
    const boost::uuids::uuid uuid = gen();
    return boost::uuids::to_string(uuid);
  }
};
