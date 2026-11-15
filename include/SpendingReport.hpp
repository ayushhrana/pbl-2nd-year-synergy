#pragma once

#include <map>
#include <string>

#include "Date.hpp"
#include "Expense.hpp"

namespace expense_tracker
{

class SpendingReport
{
public:
    SpendingReport(const Date& fromDate, const Date& toDate, long long totalSpent, int expenseCount,
                   const std::map<std::string, long long>& categoryTotals,
                   const std::map<PaymentMode, long long>& modeTotals);

    Date getFromDate() const
    {
        return fromDate;
    }

    Date getToDate() const
    {
        return toDate;
    }

    long long getTotalSpent() const
    {
        return totalSpent;
    }

    int getExpenseCount() const
    {
        return expenseCount;
    }

    int getNumberOfDays() const;
    long long getAveragePerDay() const;
    std::string getTopCategory() const;

    const std::map<std::string, long long>& getCategoryTotals() const
    {
        return categoryTotals;
    }

    const std::map<PaymentMode, long long>& getModeTotals() const
    {
        return modeTotals;
    }

private:
    Date fromDate;
    Date toDate;
    long long totalSpent;
    int expenseCount;
    std::map<std::string, long long> categoryTotals;
    std::map<PaymentMode, long long> modeTotals;
};

}
