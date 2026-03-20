#include <gtest/gtest.h>
#include <rime/registry.h>
#include <rime/setup.h>
#include <rime_api.h>

#include <quickjs.h>
#include <filesystem>
#include <string>

#include "engines/common.h"

#ifdef _WIN32
#include <windows.h>
#endif

void setJavaScriptCoreOptionsToDebug() {
#if defined(_ENABLE_JAVASCRIPTCORE)
  LOG(INFO) << "setting the undocumented JavaScriptCore options to debug";

  setenv("JSC_dumpOptions", "1", 1);  // Logs JSC runtime options at startup

  setenv("JSC_logGC", "2", 1);  // Enable GC logging: 0 = None, 1 = Basic, 2 = Verbose
  setenv("JSC_useSigillCrashAnalyzer", "1", 1);  // Enhances crash logs for JSC-related crashes

// setenv("JSC_dumpDFGDisassembly", "1", 1); // Dumps DFG JIT assembly code
// setenv("JSC_dumpFTLDisassembly", "1", 1); // Dumps FTL JIT assembly code

// setenv("JSC_showDisassembly", "1", 1); // Logs JIT-compiled code disassembly (advanced debugging)

// Seems not working: Set JSGC_MAX_HEAP_SIZE to 500MB (in bytes) before creating any JSC context
// setenv("JSGC_MAX_HEAP_SIZE", "524288000", 1); // 500MB = 500 * 1024 * 1024
#endif
}

using rime::kDefaultModules;

class GlobalEnvironment : public testing::Environment {
private:
  std::string userDataDir_;

public:
  void SetUp() override {
    std::filesystem::path path(__FILE__);
    path.remove_filename();
    userDataDir_ = path.generic_string();
    LOG(INFO) << "setting up user data dir: " << userDataDir_;


    RimeTraits traits = {
        .data_size = sizeof(RimeTraits) - sizeof((traits).data_size),
        .shared_data_dir = userDataDir_.c_str(),
        .user_data_dir = userDataDir_.c_str(),
        .distribution_name = nullptr,
        .distribution_code_name = nullptr,
        .distribution_version = nullptr,
        .app_name = "rime.test",
        .modules = nullptr,
        .min_log_level = 0,
        .log_dir = nullptr,
        .prebuilt_data_dir = ".",
        .staging_dir = ".",
    };
    rime_get_api()->setup(&traits);
    rime_get_api()->initialize(&traits);

    setJavaScriptCoreOptionsToDebug();
  }

  void TearDown() override {
    // `JsEngine<QjsValueRAII>::shutdown();` is not needed since it's called in module.cc
    rime_get_api()->finalize();
  }
};

int main(int argc, char** argv) noexcept {
  testing::InitGoogleTest(&argc, argv);
  try {
    testing::AddGlobalTestEnvironment(new GlobalEnvironment);
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "Failed to allocate GlobalEnvironment: " << e.what();
    std::exit(1);
  }

#ifdef _WIN32
  // Enables UTF-8 output in the Windows console
  // Only works when printing logs directly to console using `.\librime-qjs-tests.exe`
  // Does not work when redirecting output to a file using `.\librime-qjs-tests.exe > tests.log 2>&1`
  SetConsoleOutputCP(CP_UTF8);
#endif

  return RUN_ALL_TESTS();
}
