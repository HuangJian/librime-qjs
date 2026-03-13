#pragma once

#include <leveldb/db.h>
#include <optional>
#include <string>
#include <vector>

#include "dicts/dictionary.h"

class LevelDb final : public Dictionary {
public:
  LevelDb() = default;
  LevelDb(const LevelDb&) = delete;
  LevelDb(LevelDb&&) = delete;
  LevelDb& operator=(const LevelDb&) = delete;
  LevelDb& operator=(LevelDb&&) = delete;
  ~LevelDb() override;

  void close();
  void loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) override;
  void loadBinaryFile(const std::string& filePath) override;
  void saveToBinaryFile(const std::string& filePath) override;
  [[nodiscard]] std::optional<std::string> find(const std::string& key) const override;
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> prefixSearch(
      const std::string& prefix) const override;

private:
  leveldb::DB* ptr_ = nullptr;
  std::string txtPath_;
  ParseTextFileOptions textFileOptions_;

  std::string concatSeparator_;
};
