#include "jscode_utils.h"
#include <glog/logging.h>
#include <regex>
#include <sstream>

void replaceNewClassInstanceStatementInPlace(std::string& source,
                                            const std::string& instanceName,
                                            const std::vector<std::string>& argumentNames) {
  //  find the last statment in this format: `globalThis.sort_by_pinyin_js = new SortCandidatesByPinyinFilter()`
  std::regex pattern(R"(globalThis\.\w+\s*=\s*new\s*(\w+)\s*\(\))");
  std::sregex_iterator it(source.begin(), source.end(), pattern);
  std::sregex_iterator lastMatch = it;
  for (; it != std::sregex_iterator(); ++it) {
    lastMatch = it;
  }
  if (lastMatch == std::sregex_iterator()) {
    LOG(ERROR) << "[jsc] replaceNewClassInstanceStatementInPlace: no match found";
    return;
  }
  DLOG(INFO) << "[jsc] found class: " << lastMatch->str(1);

  std::string className = lastMatch->str(1);
  static int classIndex = 0;
  ++classIndex;
  std::string indexedClassName = className + "_" + std::to_string(classIndex);

  // replace `var SortCandidatesByPinyinFilter = class {` with: `var SortCandidatesByPinyinFilter_N = class {`
  source = std::regex_replace(source, std::regex("var\\s*" + className + R"(\s*=\s*class\s*\{)"),
                              "var " + indexedClassName + " = class {");

  //  replace `globaThis.sort_by_pinyin_js = new SortCandidatesByPinyinFilter()` with:
  // `globaThis.${instanceName} = new SortCandidatesByPinyinFilter_N(globaThis.arg0, globaThis.arg1, ...)\n})()`
  std::stringstream ss;
  ss << "globalThis." << instanceName << " = new " << indexedClassName << "(";
  for (const std::string& argumentName : argumentNames) {
    ss << "globalThis." << argumentName << ",";
  }
  std::string newStatementString = ss.str();
  newStatementString.pop_back();
  source = std::regex_replace(source, pattern, newStatementString + ")");
}

void removeExportStatementsInPlace(std::string& source) {
  size_t pos = 0;
  while ((pos = source.find("export", pos)) != std::string::npos) {
    size_t rightBracket = source.find('}', pos);
    if (rightBracket == std::string::npos) {
      rightBracket = source.length();
    } else {
      rightBracket++;
    }
    source.replace(pos, rightBracket - pos, "");
    pos = rightBracket;
  }
}
