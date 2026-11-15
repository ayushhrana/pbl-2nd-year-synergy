#pragma once

#include <vector>

#include "ExpenseManager.hpp"

namespace expense_tracker
{

class CalendarService
{
public:
    CalendarService(const ExpenseManager& manager) : manager(manager)
    {
    }

    std::vector<Expense> getExpensesOnDay(const Date& day) const;
    std::vector<Expense> getExpensesInMonth(int year, int month) const;
    std::vector<Expense> getExpensesInRange(const Date& fromDate, const Date& toDate) const;

private:
    const ExpenseManager& manager;
};

}
