#pragma once

#include <string>
#include <vector>

void replaceNewClassInstanceStatementInPlace(std::string& source,
                                             const std::string& instanceName,
                                             const std::vector<std::string>& argumentNames);

void removeExportStatementsInPlace(std::string& source);
