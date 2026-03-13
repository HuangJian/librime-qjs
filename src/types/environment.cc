#include "environment.h"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

#include <cpp-subprocess/subprocess.hpp>

#include "process_memory.h"

class ProcessManager {
public:
  struct ProcessResult {
    int exitCode{0};
    std::string output;
    std::string error;
    bool timedOut{false};
  };

  static ProcessResult runWithTimeoutAndCapture(const std::string& command,
                                                int timeoutInMilliseconds) {
    ProcessResult result;
    try {
      // [cpp-subprocess](https://github.com/arun11299/cpp-subprocess) is introduced here
      // as a light-weight (header-only) and cross-platform sub-processing library, to support
      // terminating the sub-process on timed-out.

      // boost::process is not header only, which would introduce a lot of changes in the CI building.
      // To avoid the unwanted complexity, cpp-subprocess is better for our requirement.

#ifdef __APPLE__
      // macOS: setting subprocess::shell{false} hangs the IME when running commands like 'osascript -e xxx'
      constexpr bool RUN_IN_SHELL = true;
#else
      // Windows: cpp-subprocess does not support running command in shell
      // Linux: setting subprocess::shell{true} on Ubuntu hangs the unit tests on capturing the output
      constexpr bool RUN_IN_SHELL = false;
#endif
      auto proc = std::make_shared<subprocess::Popen>(command, subprocess::output{subprocess::PIPE},
                                                      subprocess::error{subprocess::PIPE},
                                                      subprocess::shell{RUN_IN_SHELL});
      // timeoutInMilliseconds == 0 means we don't need the result of the command
      if (timeoutInMilliseconds > 0) {
        const auto waiter = std::async(std::launch::async, [proc]() {
          proc->wait();
          return true;
        });

        if (waiter.wait_for(std::chrono::milliseconds(timeoutInMilliseconds)) ==
            std::future_status::timeout) {
          result.timedOut = true;
          result.exitCode = -1;
          proc->kill();
        }

        constexpr int BUFFER_SIZE = 1024 * 1024;
        std::vector<char> buffer(BUFFER_SIZE);
        subprocess::util::read_all(proc->output(), buffer);
        result.output = std::string(buffer.begin(), buffer.end());

        if (!result.timedOut) {
          subprocess::util::read_all(proc->error(), buffer);
          result.error = std::string(buffer.begin(), buffer.end());
          result.exitCode = proc->retcode();
        }
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << "Process Exception: command = " << command << ", e.what = " << e.what();
      result.error = e.what();
      result.exitCode = -1;
    }

    return result;
  }
};

std::pair<std::string, std::string> Environment::popen(const std::string& command,
                                                       int timeoutInMilliseconds) {
  if (command.empty()) {
    return std::make_pair("", "Command is empty");
  }

  auto result = ProcessManager::runWithTimeoutAndCapture(command, timeoutInMilliseconds);
  if (result.timedOut) {
    const std::string error = "popen timed-out with command = [" + command + "] in " +
                              std::to_string(timeoutInMilliseconds) + "ms";
    return std::make_pair("", error);
  }
  if (!result.error.empty()) {
    const std::string error = "popen failed with command = [" + command +
                              "]: exitCode = " + std::to_string(result.exitCode) +
                              ", err = " + result.error;
    return std::make_pair("", error);
  }
  return std::make_pair(result.output, "");
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

bool Environment::saveFile(const std::string& path, const std::string& content) {
  if (path.empty()) {
    return false;
  }
  FILE* file = fopen(path.c_str(), "wb");
  if (file == nullptr) {
    return false;
  }
  size_t written = fwrite(content.data(), 1, content.size(), file);
  fclose(file);
  return written == content.size();
}

bool Environment::removeFile(const std::string& path) {
  if (path.empty()) {
    return false;
  }
  return std::filesystem::remove(path);
}

bool Environment::createDir(const std::string& path, bool exist_ok) {
  if (path.empty()) {
    return false;
  }
  std::error_code ec;
  if (exist_ok) {
    std::filesystem::create_directories(path, ec);
  } else {
    std::filesystem::create_directory(path, ec);
  }
  return !ec;
}

bool Environment::removeDir(const std::string& path) {
  if (path.empty()) {
    return false;
  }
  return std::filesystem::remove(path);
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
