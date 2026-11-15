#include "CalendarService.hpp"

using namespace std;

namespace expense_tracker
{

vector<Expense> CalendarService::getExpensesOnDay(const Date& day) const
{
    return manager.getExpensesBetween(day, day);
}

// the last day comes from Date::daysInMonth, so leap year February is handled
vector<Expense> CalendarService::getExpensesInMonth(int year, int month) const
{
    Date firstDay(year, month, 1);
    Date lastDay(year, month, Date::daysInMonth(year, month));
    return manager.getExpensesBetween(firstDay, lastDay);
}

vector<Expense> CalendarService::getExpensesInRange(const Date& fromDate, const Date& toDate) const
{
    return manager.getExpensesBetween(fromDate, toDate);
}

}
