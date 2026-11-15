#include "Date.hpp"

#include <ctime>

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

Date::Date()
{
    year = 2000;
    month = 1;
    day = 1;
}

Date::Date(int y, int m, int d)
{
    if (y < 1000 || y > 9999)
    {
        throw ExpenseError("Year must be between 1000 and 9999");
    }
    if (m < 1 || m > 12)
    {
        throw ExpenseError("Month must be between 1 and 12");
    }
    if (d < 1 || d > daysInMonth(y, m))
    {
        throw ExpenseError("This day does not exist in the given month");
    }
    year = y;
    month = m;
    day = d;
}

Date Date::parse(const string& text)
{
    if (text.size() != 10)
    {
        throw ExpenseError("Date must be in YYYY-MM-DD format: " + text);
    }
    if (text[4] != '-' || text[7] != '-')
    {
        throw ExpenseError("Date must be in YYYY-MM-DD format: " + text);
    }

    string yearText = text.substr(0, 4);
    string monthText = text.substr(5, 2);
    string dayText = text.substr(8, 2);

    if (!isOnlyDigits(yearText) || !isOnlyDigits(monthText) || !isOnlyDigits(dayText))
    {
        throw ExpenseError("Date must be in YYYY-MM-DD format: " + text);
    }

    int y = stoi(yearText);
    int m = stoi(monthText);
    int d = stoi(dayText);
    return Date(y, m, d);
}

bool Date::isLeapYear(int year)
{
    if (year % 400 == 0)
    {
        return true;
    }
    if (year % 100 == 0)
    {
        return false;
    }
    if (year % 4 == 0)
    {
        return true;
    }
    return false;
}

int Date::daysInMonth(int year, int month)
{
    if (month < 1 || month > 12)
    {
        throw ExpenseError("Month must be between 1 and 12");
    }
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return days[month - 1];
}

int Date::getDayNumber() const
{
    int previousYears = year - 1;
    int total = previousYears * 365;
    total = total + previousYears / 4;
    total = total - previousYears / 100;
    total = total + previousYears / 400;

    for (int m = 1; m < month; m++)
    {
        total = total + daysInMonth(year, m);
    }

    total = total + day;
    return total;
}

Date Date::fromDayNumber(int number)
{
    if (number < 1)
    {
        throw ExpenseError("Date is out of range");
    }

    int y = 1;
    while (true)
    {
        int daysInYear = 365;
        if (isLeapYear(y))
        {
            daysInYear = 366;
        }
        if (number <= daysInYear)
        {
            break;
        }
        number = number - daysInYear;
        y++;
        if (y > 9999)
        {
            throw ExpenseError("Date is out of range");
        }
    }

    int m = 1;
    while (number > daysInMonth(y, m))
    {
        number = number - daysInMonth(y, m);
        m++;
    }

    return Date(y, m, number);
}

string Date::toString() const
{
    string text = to_string(year);
    text = text + "-";
    if (month < 10)
    {
        text = text + "0";
    }
    text = text + to_string(month);
    text = text + "-";
    if (day < 10)
    {
        text = text + "0";
    }
    text = text + to_string(day);
    return text;
}

Date Date::addDays(int days) const
{
    int newNumber = getDayNumber() + days;
    return fromDayNumber(newNumber);
}

// if the day does not exist in the new month, the last day of that month is used
// example: 31 January + 1 month = 28 February
Date Date::addMonths(int months) const
{
    int totalMonths = year * 12 + (month - 1) + months;
    if (totalMonths < 0)
    {
        throw ExpenseError("Date is out of range");
    }
    int newYear = totalMonths / 12;
    int newMonth = totalMonths % 12 + 1;
    if (newYear < 1000 || newYear > 9999)
    {
        throw ExpenseError("Date is out of range");
    }

    int newDay = day;
    int lastDay = daysInMonth(newYear, newMonth);
    if (newDay > lastDay)
    {
        newDay = lastDay;
    }
    return Date(newYear, newMonth, newDay);
}

Date Date::today()
{
    time_t now = time(nullptr);
    tm* local = localtime(&now);
    return Date(local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
}

string Date::getMonthName(int month)
{
    string names[12] = {"January", "February", "March", "April", "May", "June",
                        "July", "August", "September", "October", "November", "December"};
    if (month < 1 || month > 12)
    {
        throw ExpenseError("Month must be between 1 and 12");
    }
    return names[month - 1];
}

int Date::getDayOfWeek() const
{
    return (getDayNumber() - 1) % 7;
}

string Date::getDayName() const
{
    string names[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    return names[getDayOfWeek()];
}

}
