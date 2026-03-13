#pragma once

#include <marisa.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "dicts/dictionary.h"

namespace rime {

class Trie final : public Dictionary {
  marisa::Trie trie_;
  std::vector<std::string> data_;
  std::string concatSeparator_;

protected:
  marisa::Trie& getTrie() { return trie_; }
  std::vector<std::string>& getData() { return data_; }

  // Enhanced I/O utilities
  struct IOUtil {
    static void writeSizeAndData(std::ostream& out, const std::string& data) {
      const size_t len = data.length();
      out.write(reinterpret_cast<const char*>(&len), sizeof(len));
      out.write(data.data(), static_cast<std::streamsize>(len));
    }

    static std::string readSizedString(std::istream& in) {
      size_t len = 0;
      in.read(reinterpret_cast<char*>(&len), sizeof(len));
      std::string result(len, '\0');
      in.read(result.data(), static_cast<std::streamsize>(len));
      return result;
    }

    static void writeVectorData(std::ostream& out, const std::vector<std::string>& data) {
      const size_t size = data.size();
      out.write(reinterpret_cast<const char*>(&size), sizeof(size));
      for (const auto& str : data) {
        writeSizeAndData(out, str);
      }
    }

    static void readVectorData(std::istream& in, std::vector<std::string>& data) {
      size_t size = 0;
      in.read(reinterpret_cast<char*>(&size), sizeof(size));
      data.resize(size);
      for (auto& str : data) {
        str = readSizedString(in);
      }
    }
  };

  class ScopedTempFile {
    std::filesystem::path path_;

  public:
    explicit ScopedTempFile(const std::filesystem::path& base) : path_(base.string() + ".temp") {}
    ~ScopedTempFile() { std::filesystem::remove(path_); }

    ScopedTempFile(const ScopedTempFile&) = delete;
    ScopedTempFile& operator=(const ScopedTempFile&) = delete;
    ScopedTempFile(ScopedTempFile&&) = delete;
    ScopedTempFile& operator=(ScopedTempFile&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return path_; }
  };

public:
  void loadBinaryFile(const std::string& filePath) override;
  void loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) override;
  void saveToBinaryFile(const std::string& filePath) override;
  [[nodiscard]] std::optional<std::string> find(const std::string& key) const override;
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> prefixSearch(
      const std::string& prefix) const override;

  void add(const std::string& key, const std::string& value);
  void build(const std::unordered_map<std::string, std::string>& map);
  [[nodiscard]] bool contains(std::string_view key) const;
};

}  // namespace rime
