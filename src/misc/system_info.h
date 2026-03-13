#pragma once

#include <string>

class SystemInfo {
public:
  static std::string getOSName();
  static std::string getOSVersion();
  static std::string getArchitecture();

private:
#ifdef _WIN32
  static std::string getWindowsVersion();
  static std::string getWindowsArchitecture();
#endif

#ifdef __linux__
  static bool detectLinuxDistro();
  static std::string getLinuxVersion();
  static bool checkOsRelease();
  static std::string readOsRelease();
  static bool checkLsbRelease();
  static std::string readLsbRelease();
  static bool checkRedHatRelease();
  static std::string readRedHatRelease();
  static bool checkDebianVersion();
  static std::string readDebianVersion();
  static bool checkSuseRelease();
  static std::string readSuseRelease();
  static bool checkArchRelease();
  static std::string readArchRelease();
  static void trimQuotes(std::string& str);
#endif

#ifdef __APPLE__
  static std::string getMacOSVersion();
#endif
};
