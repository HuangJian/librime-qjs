#include "dicts/trie.h"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

namespace rime {

void Trie::loadBinaryFile(const std::string& filePath) {
  const boost::interprocess::file_mapping mapping(std::string(filePath).c_str(),
                                                  boost::interprocess::read_only);
  const boost::interprocess::mapped_region region(mapping, boost::interprocess::read_only);

  const char* current = static_cast<const char*>(region.get_address());
  const char* end = current + region.get_size();

  // Read data vector size
  size_t dataSize = 0;
  std::memcpy(&dataSize, current, sizeof(dataSize));
  current += sizeof(dataSize);

  data_.resize(dataSize);

  // Read strings
  for (size_t i = 0; i < dataSize && current < end; ++i) {
    size_t strLen = 0;
    std::memcpy(&strLen, current, sizeof(strLen));
    current += sizeof(strLen);

    if (current + strLen > end) {
      throw std::runtime_error("Corrupted data file");
    }

    data_[i].assign(current, strLen);
    current += strLen;
  }

  // Read and load trie
  size_t trieSize = 0;
  std::memcpy(&trieSize, current, sizeof(trieSize));
  current += sizeof(trieSize);

  off_t offset = current - static_cast<const char*>(region.get_address());
#ifdef _WIN32
  auto native_handle = mapping.get_mapping_handle().handle;
  SetFilePointer(reinterpret_cast<HANDLE>(native_handle), offset, nullptr, FILE_BEGIN);
  int fd = _open_osfhandle(reinterpret_cast<intptr_t>(native_handle), _O_RDONLY);
  trie_.read(fd);
  _close(fd);
#else
  ::lseek(mapping.get_mapping_handle().handle, offset, SEEK_SET);
  trie_.read(mapping.get_mapping_handle().handle);
#endif

  auto optSeparator = find(MAGIC_KEY_TO_STORE_CONCAT_SEPARATOR);
  concatSeparator_ = optSeparator.has_value() ? optSeparator.value() : "";
}

void Trie::loadTextFile(const std::string& txtPath, const ParseTextFileOptions& options) {
  std::unordered_map<std::string, std::string> map = parseTextFile(txtPath, options);
  if (options.onDuplicatedKey == OnDuplicatedKey::Concat) {
    map[MAGIC_KEY_TO_STORE_CONCAT_SEPARATOR] = options.concatSeparator;
    concatSeparator_ = options.concatSeparator;
  }
  build(map);
}

void Trie::saveToBinaryFile(const std::string& filePath) {
  std::ofstream file(filePath, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open file for writing " + filePath);
  }

  IOUtil::writeVectorData(file, data_);

  // Handle trie data
  ScopedTempFile tempFile{filePath};
  trie_.save(tempFile.path().string().c_str());

  std::ifstream trieFile(tempFile.path(), std::ios::binary | std::ios::ate);
  const size_t trieSize = trieFile.tellg();
  trieFile.seekg(0);

  file.write(reinterpret_cast<const char*>(&trieSize), sizeof(trieSize));
  file << trieFile.rdbuf();
}

void Trie::add(const std::string& key, const std::string& value) {
  marisa::Keyset keyset;
  keyset.push_back(key.c_str(), key.length());

  // Build the trie
  trie_.build(keyset, MARISA_BINARY_TAIL);  // UTF-8 support

  // Get the ID for the key
  marisa::Agent agent;
  agent.set_query(key.c_str(), key.length());

  if (trie_.lookup(agent)) {
    const std::size_t id = agent.key().id();

    // Resize data vector if necessary
    if (id >= data_.size()) {
      data_.resize(id + 1);
    }

    // Store the associated data
    data_[id] = value;
  } else {
    throw std::runtime_error("Failed to add key-value pair");
  }
}

void Trie::build(const std::unordered_map<std::string, std::string>& map) {
  marisa::Keyset keyset;

  // First, add all keys to the keyset
  for (const auto& [key, _] : map) {
    keyset.push_back(key.c_str(), key.length());
  }

  // Build the trie
  trie_.build(keyset, MARISA_BINARY_TAIL);  // UTF-8 support

  // Resize data vector to accommodate all values
  data_.resize(map.size());

  // Store all values
  for (const auto& [key, value] : map) {
    marisa::Agent agent;
    agent.set_query(key.c_str(), key.length());

    if (trie_.lookup(agent)) {
      data_[agent.key().id()] = value;
    } else {
      throw std::runtime_error("Failed to add key-value pair");
    }
  }
}

std::optional<std::string> Trie::find(const std::string& key) const {
  marisa::Agent agent;
  agent.set_query(key.data(), key.length());

  if (trie_.lookup(agent)) {
    std::size_t id = agent.key().id();
    if (id < data_.size()) {
      return data_[id];
    }
  }
  return std::nullopt;
}

bool Trie::contains(std::string_view key) const {
  marisa::Agent agent;
  agent.set_query(key.data(), key.length());
  return trie_.lookup(agent);
}

std::vector<std::pair<std::string, std::string>> Trie::prefixSearch(
    const std::string& prefix) const {
  std::vector<std::pair<std::string, std::string>> results;
  marisa::Agent agent;
  agent.set_query(prefix.data(), prefix.length());

  while (trie_.predictive_search(agent)) {
    std::string key(agent.key().ptr(), agent.key().length());
    if (const std::size_t id = agent.key().id(); id < data_.size()) {
      auto value = data_[id];
      if (!concatSeparator_.empty()) {
        auto arr = split(value, concatSeparator_);
        for (auto& item : arr) {
          results.emplace_back(key, item);
        }
      } else {
        results.emplace_back(key, value);
      }
    }
  }
  return results;
}

}  // namespace rime
