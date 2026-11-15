#pragma once

#include <string>

namespace expense_tracker
{

const long long MAX_AMOUNT = 1000000000000LL;

void checkAmount(long long paise);
long long parseHundredths(const std::string& text);
long long rupeesToPaise(const std::string& text);
std::string paiseToRupees(long long paise);

}
