#include "RecurringExpense.hpp"

#include "Errors.hpp"
#include "Money.hpp"

using namespace std;

namespace expense_tracker
{

string frequencyToString(Frequency frequency)
{
    if (frequency == DAILY)
    {
        return "daily";
    }
    if (frequency == WEEKLY)
    {
        return "weekly";
    }
    return "monthly";
}

RecurringExpense::RecurringExpense(const ExpenseData& data, Frequency frequency, const Date& startDate)
{
    checkAmount(data.amount);
    this->data = data;
    this->frequency = frequency;
    this->startDate = startDate;
    endDateGiven = false;
}

RecurringExpense::RecurringExpense(const ExpenseData& data, Frequency frequency, const Date& startDate, const Date& endDate)
{
    checkAmount(data.amount);
    if (endDate < startDate)
    {
        throw ExpenseError("End date is before the start date");
    }
    this->data = data;
    this->frequency = frequency;
    this->startDate = startDate;
    this->endDate = endDate;
    endDateGiven = true;
}

// index 0 is the start date
// monthly dates are always counted from the start date, so a rule that starts on
// 31 January gives 31 Jan, 28 Feb, 31 Mar, 30 Apr and so on
Date RecurringExpense::getOccurrenceDate(int index) const
{
    if (index < 0)
    {
        throw ExpenseError("Occurrence number cannot be negative");
    }

    if (frequency == DAILY)
    {
        return startDate.addDays(index);
    }
    if (frequency == WEEKLY)
    {
        return startDate.addDays(index * 7);
    }
    return startDate.addMonths(index);
}

ExpenseData RecurringExpense::makeExpenseData(const Date& date) const
{
    ExpenseData result = data;
    result.date = date;
    return result;
}

}
