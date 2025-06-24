#include "environment.h"

#include <glog/logging.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>

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

//Structure to hold pipe data and synchronization
struct PipeData {
  std::string output;
  std::mutex mutex;
  std::condition_variable cv;
  bool finished = false;
};

// Reads pipe output into a string (runs in worker thread)
void readPipe(FILE* pipe, PipeData& data) {
  constexpr size_t READ_BUFFER_SIZE = 128;
  char buffer[READ_BUFFER_SIZE];
  auto* ptrBuffer = static_cast<char*>(buffer);
  while (fgets(ptrBuffer, sizeof(buffer), pipe) != nullptr) {
    std::lock_guard lock(data.mutex);
    data.output += ptrBuffer;
  }
  {
    std::lock_guard lock(data.mutex);
    data.finished = true;
  }
  data.cv.notify_one();  // Notify parent thread
}

// Waits for thread to finish or timeout
template <typename Rep, typename Period>
bool joinWithTimeout(std::thread& thread,
                     PipeData& data,
                     const std::chrono::duration<Rep, Period>& timeout) {
  std::unique_lock lock(data.mutex);

  // Wait until either:
  // - The thread finishes (data.finished == true), or
  // - The timeout is reached
  while (!data.finished) {
    if (data.cv.wait_for(lock, timeout) == std::cv_status::timeout) {
      return false;  // Timeout reached
    }
  }
  lock.unlock();

  if (thread.joinable()) {
    thread.join();
  }
  return true;  // Thread finished normally
}

std::pair<std::string, std::string> Environment::popen(const std::string& command,
                                                       int timeoutInMilliseconds) {
  if (command.empty()) {
    return std::make_pair("", "Command is empty");
  }

  FILE* pipe = popenx(command);
  if (pipe == nullptr) {
    return std::make_pair("", "Failed to run command: " + command);
  }

  PipeData pipeData;
  std::thread worker(readPipe, pipe, std::ref(pipeData));
  const bool completed =
      joinWithTimeout(worker, pipeData, std::chrono::milliseconds(timeoutInMilliseconds));
  if (!completed) {
    pclosex(pipe);  // Forcefully terminate
    if (worker.joinable()) {
      worker.join();
    }
    const std::string error = "popen timed-out with command = [" + command + "] in " +
                              std::to_string(timeoutInMilliseconds) + "ms";
    return std::make_pair("", error);
  }

  const int status = pclosex(pipe);
  if (status != 0) {
    return std::make_pair("", "Command failed with status: " + std::to_string(status));
  }

  return std::make_pair(pipeData.output, "");
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
