#include "Money.hpp"

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

void checkAmount(long long paise)
{
    if (paise <= 0)
    {
        throw ExpenseError("Amount must be greater than zero");
    }
    if (paise > MAX_AMOUNT)
    {
        throw ExpenseError("Amount is too big");
    }
}

long long parseHundredths(const string& text)
{
    string wholePart = text;
    string decimalPart = "";
    bool hasDot = false;

    size_t dotPosition = text.find('.');
    if (dotPosition != string::npos)
    {
        hasDot = true;
        wholePart = text.substr(0, dotPosition);
        decimalPart = text.substr(dotPosition + 1);
    }

    if (!isOnlyDigits(wholePart) || wholePart.size() > 12)
    {
        throw ExpenseError("Amount must be like 120 or 120.50 : " + text);
    }
    if (hasDot)
    {
        if (!isOnlyDigits(decimalPart) || decimalPart.size() > 2)
        {
            throw ExpenseError("Amount can have only 2 digits after the dot : " + text);
        }
    }

    long long paise = stoll(wholePart) * 100;
    if (decimalPart.size() == 1)
    {
        paise = paise + (decimalPart[0] - '0') * 10;
    }
    if (decimalPart.size() == 2)
    {
        paise = paise + (decimalPart[0] - '0') * 10 + (decimalPart[1] - '0');
    }

    return paise;
}

long long rupeesToPaise(const string& text)
{
    long long paise = parseHundredths(text);
    checkAmount(paise);
    return paise;
}

string paiseToRupees(long long paise)
{
    string sign = "";
    if (paise < 0)
    {
        sign = "-";
        paise = -paise;
    }

    long long rupees = paise / 100;
    long long remainingPaise = paise % 100;

    string text = sign + "Rs. " + to_string(rupees) + ".";
    if (remainingPaise < 10)
    {
        text = text + "0";
    }
    text = text + to_string(remainingPaise);
    return text;
}

}
