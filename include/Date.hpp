#pragma once

#include <string>

namespace expense_tracker
{

class Date
{
public:
    Date();
    Date(int y, int m, int d);

    static Date parse(const std::string& text);
    static bool isLeapYear(int year);
    static int daysInMonth(int year, int month);
    static Date fromDayNumber(int number);
    static Date today();
    static std::string getMonthName(int month);

    int getYear() const
    {
        return year;
    }

    int getMonth() const
    {
        return month;
    }

    int getDay() const
    {
        return day;
    }

    int getDayNumber() const;
    int getDayOfWeek() const;
    std::string getDayName() const;
    std::string toString() const;

    Date addDays(int days) const;
    Date addMonths(int months) const;

    bool operator==(const Date& other) const
    {
        if (year == other.year && month == other.month && day == other.day)
        {
            return true;
        }
        return false;
    }

    bool operator!=(const Date& other) const
    {
        return !(*this == other);
    }

    bool operator<(const Date& other) const
    {
        if (year != other.year)
        {
            return year < other.year;
        }
        if (month != other.month)
        {
            return month < other.month;
        }
        return day < other.day;
    }

    bool operator>(const Date& other) const
    {
        return other < *this;
    }

    bool operator<=(const Date& other) const
    {
        return !(other < *this);
    }

    bool operator>=(const Date& other) const
    {
        return !(*this < other);
    }

private:
    int year;
    int month;
    int day;
};

}
