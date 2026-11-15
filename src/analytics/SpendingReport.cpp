#include "SpendingReport.hpp"

using namespace std;

namespace expense_tracker
{

SpendingReport::SpendingReport(const Date& fromDate, const Date& toDate, long long totalSpent, int expenseCount,
                               const map<string, long long>& categoryTotals,
                               const map<PaymentMode, long long>& modeTotals)
{
    this->fromDate = fromDate;
    this->toDate = toDate;
    this->totalSpent = totalSpent;
    this->expenseCount = expenseCount;
    this->categoryTotals = categoryTotals;
    this->modeTotals = modeTotals;
}

int SpendingReport::getNumberOfDays() const
{
    return toDate.getDayNumber() - fromDate.getDayNumber() + 1;
}

long long SpendingReport::getAveragePerDay() const
{
    return totalSpent / getNumberOfDays();
}

string SpendingReport::getTopCategory() const
{
    string topName = "";
    long long topTotal = 0;

    map<string, long long>::const_iterator it;
    for (it = categoryTotals.begin(); it != categoryTotals.end(); ++it)
    {
        if (it->second > topTotal)
        {
            topTotal = it->second;
            topName = it->first;
        }
    }
    return topName;
}

}
