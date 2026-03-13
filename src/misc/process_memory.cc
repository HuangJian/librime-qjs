#include "process_memory.h"
#include <iostream>

#ifdef __linux__
#include <unistd.h>

#include <fstream>

void getMemoryUsage(size_t& vm_usage, size_t& resident_set) {
  vm_usage = 0;
  resident_set = 0;

  std::ifstream statm("/proc/self/statm");
  if (statm) {
    size_t size, resident, share, text, lib, data, dt;
    statm >> size >> resident >> share >> text >> lib >> data >> dt;

    long page_size = sysconf(_SC_PAGESIZE);  // in bytes
    vm_usage = size * page_size;
    resident_set = resident * page_size;
  } else {
    std::cerr << "Failed to open /proc/self/statm" << std::endl;
  }
}

#elif _WIN32
// do not sort the following includes, otherwise it will cause the compilation error in windows.
// clang-format off
#include <windows.h>
#include <Psapi.h>
// clang-format on

void getMemoryUsage(size_t& vm_usage, size_t& resident_set) {
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    vm_usage = pmc.WorkingSetSize;
    resident_set = pmc.PagefileUsage;
  } else {
    std::cerr << "Failed to get process memory info" << std::endl;
  }
}

#elif __APPLE__
#include <mach/mach.h>
#include <unistd.h>  // For getpid()

void getMemoryUsage(size_t& vmUsage, size_t& residentSet) {
  vmUsage = 0;
  residentSet = 0;

  task_t task = MACH_PORT_NULL;
  struct task_basic_info tInfo{};
  mach_msg_type_number_t tInfoCount = TASK_BASIC_INFO_COUNT;

  if (task_for_pid(current_task(), ::getpid(), &task) != KERN_SUCCESS) {
    std::cerr << "Failed to get task for process\n";
    return;
  }

  if (task_info(task, TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&tInfo), &tInfoCount) !=
      KERN_SUCCESS) {
    std::cerr << "Failed to get task info\n";
    return;
  }

  vmUsage = tInfo.virtual_size;       // Virtual memory size
  residentSet = tInfo.resident_size;  // Resident set size
}

#else
#error "Platform not supported"
#endif
