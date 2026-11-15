#pragma once

#include <string>
#include <vector>

namespace expense_tracker
{

std::string toLowerCase(const std::string& text);
std::string trimSpaces(const std::string& text);
bool isSameText(const std::string& first, const std::string& second);
bool containsText(const std::string& text, const std::string& part);
bool isOnlyDigits(const std::string& text);
std::string encodeField(const std::string& text);
std::string decodeField(const std::string& text);
std::vector<std::string> splitText(const std::string& text, char separator);

}
