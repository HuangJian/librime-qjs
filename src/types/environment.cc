#include "environment.h"

#include <glog/logging.h>

#include <chrono>
#include <cstdio>
#include <string>

#include "process_memory.hpp"

#ifdef _WIN32
#include <windows.h>
static FILE* popenx(const std::string& command) {
  return _popen(command.c_str(), "r");
}

static int pclosex(FILE* pipe) {
  return _pclose(pipe);
}
#else
static FILE* popenx(const std::string& command) {
  return ::popen(command.c_str(), "r");
}

static int pclosex(FILE* pipe) {
  return ::pclose(pipe);
}
#endif
std::pair<std::string, std::string> Environment::popen(const std::string& command,
                                                       int timeoutInMilliseconds) {
  if (command.empty()) {
    return std::make_pair("", "Command is empty");
  }

  FILE* pipe = popenx(command);
  if (pipe == nullptr) {
    return std::make_pair("", "Failed to run command: " + command);
  }

  std::string output;
  constexpr size_t READ_BUFFER_SIZE = 128;
  char buffer[READ_BUFFER_SIZE];
  auto* ptrBuffer = static_cast<char*>(buffer);
  auto startTime = std::chrono::steady_clock::now();

  while (true) {
    if (fgets(ptrBuffer, sizeof(ptrBuffer), pipe) != nullptr) {
      output += ptrBuffer;
    } else if (feof(pipe) != 0) {
      break;  // End of output
    } else if (ferror(pipe) != 0) {
      pclosex(pipe);
      return std::make_pair("", "Error reading command output");
    }

    // checking timeout in this while loop is not precise, a two seconds delay is detected.
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    if (elapsedTime.count() >= timeoutInMilliseconds) {
      pclosex(pipe);
      const std::string error = "popen timed-out with command = [" + command + "] in " +
                                std::to_string(timeoutInMilliseconds) + "ms";
      return std::make_pair("", error);
    }
  }

  int returnCode = pclosex(pipe);
  if (returnCode != 0) {
    return std::make_pair("", "Command failed with return code: " + std::to_string(returnCode));
  }

  return std::make_pair(output, "");
}

std::string Environment::formatMemoryUsage(size_t usage) {
  constexpr size_t KILOBYTE = 1024;
  return usage > KILOBYTE * KILOBYTE ? std::to_string(usage / KILOBYTE / KILOBYTE) + "M"
                                     : std::to_string(usage / KILOBYTE) + "K";
}

std::string Environment::loadFile(const std::string& path) {
  if (path.empty()) {
    return "";
  }

  FILE* file = fopen(path.c_str(), "rb");
  if (file == nullptr) {
    return "";
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  std::string content;
  content.resize(size);
  fread(content.data(), 1, size, file);
  fclose(file);

  return content;
}

bool Environment::fileExists(const std::string& path) {
  return std::filesystem::exists(path);
}

std::string Environment::getRimeInfo() {
  size_t vmUsage = 0;
  size_t residentSet = 0;  // memory usage in bytes
  getMemoryUsage(vmUsage, residentSet);

  std::stringstream ss{};
  ss << "libRime v" << rime_get_api()->get_version() << " | "
     << "libRime-qjs v" << RIME_QJS_VERSION << " | "
     << "Process RSS Mem: " << formatMemoryUsage(residentSet);

  return ss.str();
}
