#pragma once

#include <cstdint>  // needed for Linux building
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

constexpr auto MAGIC_KEY_TO_STORE_CONCAT_SEPARATOR = "MAGIC_KEY_TO_STORE_CONCAT_SEPARATOR";

enum class OnDuplicatedKey : std::uint8_t {
  Overwrite,
  Skip,
  Concat,
};

struct ParseTextFileOptions {
  std::string delimiter = "\t";
  std::string comment = "#";
  size_t lines = 0;
  bool isReversed = false;
  std::string charsToRemove = "\r";
  OnDuplicatedKey onDuplicatedKey = OnDuplicatedKey::Overwrite;
  std::string concatSeparator = "$|$";
};

class Dictionary {
public:
  virtual ~Dictionary() = default;

  virtual void loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) = 0;
  virtual void loadBinaryFile(const std::string& filePath) = 0;
  virtual void saveToBinaryFile(const std::string& filePath) = 0;
  [[nodiscard]] virtual std::optional<std::string> find(const std::string& key) const = 0;
  [[nodiscard]] virtual std::vector<std::pair<std::string, std::string>> prefixSearch(
      const std::string& prefix) const = 0;

  static std::unordered_map<std::string, std::string> parseTextFile(
      const std::string& path,
      const ParseTextFileOptions& options);

protected:
  static std::vector<std::string> split(const std::string& str, const std::string& delimiters);

private:
  static void removeChars(std::string& str, const std::string& charsToRemove);
};
