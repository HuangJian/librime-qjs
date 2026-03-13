#pragma once

#include <JavaScriptCore/JavaScript.h>
#include <filesystem>
#include <string>
#include <vector>

class JscCodeLoader {
public:
  static JSObjectRef createInstanceOfIifeBundledModule(JSContextRef ctx,
                                                       const std::string& baseFolderPath,
                                                       const std::string& moduleName,
                                                       const std::vector<JSValueRef>& args,
                                                       JSValueRef* exception);

  static JSValueRef loadEsmBundledModuleToGlobalThis(JSContextRef ctx,
                                                     const std::string& baseFolderPath,
                                                     const std::string& moduleName,
                                                     JSValueRef* exception);

  static JSValueRef getExportedClassHavingMethodNameInModule(JSContextRef ctx,
                                                             JSValueRef moduleObj,
                                                             const char* methodName);
  static JSValueRef getExportedClassByNameInModule(JSContextRef ctx,
                                                   JSValueRef moduleObj,
                                                   const char* className);
  static JSValueRef getMethodByNameInClass(JSContextRef ctx,
                                           JSValueRef classObj,
                                           const char* methodName);

private:
  static std::pair<std::string, std::filesystem::path> loadModuleSource(
      JSContextRef ctx,
      const std::string& baseFolderPath,
      const std::string& moduleName);
};
