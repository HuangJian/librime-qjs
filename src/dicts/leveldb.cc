#include "dicts/leveldb.h"

#include <leveldb/write_batch.h>
#include <stdexcept>

LevelDb::~LevelDb() {
  if (ptr_ != nullptr) {
    delete ptr_;
    ptr_ = nullptr;
  }
}

void LevelDb::close() {
  if (ptr_ != nullptr) {
    delete ptr_;
    ptr_ = nullptr;
  }
}

void LevelDb::loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) {
  txtPath_ = txtPath;
  textFileOptions_ = options;
}

void LevelDb::loadBinaryFile(const std::string& filePath) {
  leveldb::Options options;
  options.create_if_missing = false;
  options.paranoid_checks = false;  // Disable expensive checks
  options.reuse_logs = true;        // Reuse existing log files
  leveldb::DB::Open(options, filePath, &ptr_);

  auto optSeparator = find(MAGIC_KEY_TO_STORE_CONCAT_SEPARATOR);
  concatSeparator_ = optSeparator.has_value() ? optSeparator.value() : "";
}

void LevelDb::saveToBinaryFile(const std::string& filePath) {
  if (txtPath_.empty()) {
    throw std::runtime_error("No text file loaded.");
  }
  if (ptr_ != nullptr) {
    throw std::runtime_error("LevelDb already loaded.");
  }

  std::unordered_map<std::string, std::string> map = parseTextFile(txtPath_, textFileOptions_);

  if (textFileOptions_.onDuplicatedKey == OnDuplicatedKey::Concat) {
    map[MAGIC_KEY_TO_STORE_CONCAT_SEPARATOR] = textFileOptions_.concatSeparator;
  }

  leveldb::WriteBatch batch;
  for (const auto& [key, val] : map) {
    batch.Put(key, val);
  }

  leveldb::Options options;
  options.create_if_missing = true;
  options.error_if_exists = false;
  leveldb::DB::Open(options, filePath, &ptr_);
  ptr_->Write(leveldb::WriteOptions(), &batch);
  batch.Clear();
}

std::optional<std::string> LevelDb::find(const std::string& key) const {
  if (ptr_ == nullptr) {
    throw std::runtime_error("LevelDb not loaded.");
  }
  std::string value;
  auto status = ptr_->Get(leveldb::ReadOptions(), key, &value);
  return status.ok() ? std::make_optional(value) : std::nullopt;
}

std::vector<std::pair<std::string, std::string>> LevelDb::prefixSearch(
    const std::string& prefix) const {
  if (ptr_ == nullptr) {
    throw std::runtime_error("LevelDb not loaded.");
  }

  std::vector<std::pair<std::string, std::string>> results;
  leveldb::Iterator* it = ptr_->NewIterator(leveldb::ReadOptions());
  for (it->Seek(prefix); it->Valid(); it->Next()) {
    std::string key = it->key().ToString();
    if (key.find(prefix) != 0) {
      break;
    }
    std::string value = it->value().ToString();

    if (!concatSeparator_.empty()) {
      auto arr = split(value, concatSeparator_);
      for (auto& item : arr) {
        results.emplace_back(key, item);
      }
    } else {
      results.emplace_back(key, value);
    }
  }
  delete it;
  return results;
}
