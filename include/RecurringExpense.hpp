#pragma once

#include <string>

#include "Expense.hpp"

namespace expense_tracker
{

enum Frequency
{
    DAILY,
    WEEKLY,
    MONTHLY
};

std::string frequencyToString(Frequency frequency);

class RecurringExpense
{
public:
    RecurringExpense(const ExpenseData& data, Frequency frequency, const Date& startDate);
    RecurringExpense(const ExpenseData& data, Frequency frequency, const Date& startDate, const Date& endDate);

    Date getOccurrenceDate(int index) const;
    ExpenseData makeExpenseData(const Date& date) const;

    bool isAfterEnd(const Date& date) const
    {
        if (endDateGiven && date > endDate)
        {
            return true;
        }
        return false;
    }

    ExpenseData getData() const
    {
        return data;
    }

    Frequency getFrequency() const
    {
        return frequency;
    }

    Date getStartDate() const
    {
        return startDate;
    }

    bool hasEndDate() const
    {
        return endDateGiven;
    }

    Date getEndDate() const
    {
        return endDate;
    }

private:
    ExpenseData data;
    Frequency frequency;
    Date startDate;
    bool endDateGiven;
    Date endDate;
};

}
