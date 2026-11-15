#include "AnalyticsService.hpp"

#include <algorithm>
#include <vector>

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

long long AnalyticsService::totalBetween(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    long long total = 0;
    for (int i = 0; i < (int)items.size(); i++)
    {
        total = total + items[i].getAmount();
    }
    return total;
}

// a map keeps the categories in sorted order
map<string, long long> AnalyticsService::totalByCategory(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    map<string, long long> totals;
    for (int i = 0; i < (int)items.size(); i++)
    {
        string name = items[i].getCategory();
        totals[name] = totals[name] + items[i].getAmount();
    }
    return totals;
}

map<PaymentMode, long long> AnalyticsService::totalByMode(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    map<PaymentMode, long long> totals;
    for (int i = 0; i < (int)items.size(); i++)
    {
        PaymentMode mode = items[i].getMode();
        totals[mode] = totals[mode] + items[i].getAmount();
    }
    return totals;
}

BudgetStatus AnalyticsService::budgetStatus(const Budget& budget, const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    long long spent = 0;
    for (int i = 0; i < (int)items.size(); i++)
    {
        if (budget.getCategory().empty() || isSameText(items[i].getCategory(), budget.getCategory()))
        {
            spent = spent + items[i].getAmount();
        }
    }

    BudgetStatus status;
    status.limit = budget.getLimit();
    status.spent = spent;
    status.remaining = budget.getLimit() - spent;
    if (spent > budget.getLimit())
    {
        status.exceeded = true;
    }
    else
    {
        status.exceeded = false;
    }
    return status;
}

static bool isTotalGreater(const CategoryTotal& first, const CategoryTotal& second)
{
    if (first.total != second.total)
    {
        return first.total > second.total;
    }
    return first.category < second.category;
}

map<string, long long> AnalyticsService::dailySummary(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    map<string, long long> summary;
    for (int i = 0; i < (int)items.size(); i++)
    {
        string key = items[i].getDate().toString();
        summary[key] = summary[key] + items[i].getAmount();
    }
    return summary;
}

map<string, long long> AnalyticsService::weeklySummary(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    map<string, long long> summary;
    for (int i = 0; i < (int)items.size(); i++)
    {
        Date date = items[i].getDate();
        Date monday = date.addDays(-date.getDayOfWeek());
        string key = monday.toString();
        summary[key] = summary[key] + items[i].getAmount();
    }
    return summary;
}

map<string, long long> AnalyticsService::monthlySummary(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    map<string, long long> summary;
    for (int i = 0; i < (int)items.size(); i++)
    {
        string key = items[i].getDate().toString().substr(0, 7);
        summary[key] = summary[key] + items[i].getAmount();
    }
    return summary;
}

vector<CategoryTotal> AnalyticsService::topCategories(const Date& fromDate, const Date& toDate, int n) const
{
    if (n <= 0)
    {
        throw ExpenseError("n must be greater than zero");
    }

    map<string, long long> totals = totalByCategory(fromDate, toDate);

    vector<CategoryTotal> list;
    map<string, long long>::iterator it;
    for (it = totals.begin(); it != totals.end(); ++it)
    {
        CategoryTotal item;
        item.category = it->first;
        item.total = it->second;
        list.push_back(item);
    }

    sort(list.begin(), list.end(), isTotalGreater);

    if ((int)list.size() > n)
    {
        list.resize(n);
    }
    return list;
}

PeriodComparison AnalyticsService::comparePeriods(const Date& firstFrom, const Date& firstTo,
                                                  const Date& secondFrom, const Date& secondTo) const
{
    PeriodComparison result;
    result.firstTotal = totalBetween(firstFrom, firstTo);
    result.secondTotal = totalBetween(secondFrom, secondTo);
    result.difference = result.secondTotal - result.firstTotal;

    if (result.difference > 0)
    {
        result.trend = "increased";
    }
    else if (result.difference < 0)
    {
        result.trend = "decreased";
    }
    else
    {
        result.trend = "same";
    }

    if (result.firstTotal > 0)
    {
        result.hasPercent = true;
        result.percentChange = result.difference * 100 / result.firstTotal;
    }
    return result;
}

vector<long long> AnalyticsService::makeDailyPrefixSums(const Date& fromDate, const Date& toDate) const
{
    vector<Expense> items = manager.getExpensesBetween(fromDate, toDate);

    int numberOfDays = toDate.getDayNumber() - fromDate.getDayNumber() + 1;
    if (numberOfDays > 40000)
    {
        throw ExpenseError("Date range is too long");
    }

    vector<long long> dayTotals(numberOfDays, 0);
    for (int i = 0; i < (int)items.size(); i++)
    {
        int dayIndex = items[i].getDate().getDayNumber() - fromDate.getDayNumber();
        dayTotals[dayIndex] = dayTotals[dayIndex] + items[i].getAmount();
    }

    vector<long long> prefix(numberOfDays + 1, 0);
    for (int k = 1; k <= numberOfDays; k++)
    {
        prefix[k] = prefix[k - 1] + dayTotals[k - 1];
    }
    return prefix;
}

long long AnalyticsService::totalFromPrefix(const vector<long long>& prefix, int startDay, int endDay) const
{
    int numberOfDays = (int)prefix.size() - 1;
    if (startDay < 0 || endDay >= numberOfDays || startDay > endDay)
    {
        throw ExpenseError("Day numbers are not valid for this range");
    }
    return prefix[endDay + 1] - prefix[startDay];
}

SpendingReport AnalyticsService::makeReport(const Date& fromDate, const Date& toDate) const
{
    int count = (int)manager.getExpensesBetween(fromDate, toDate).size();
    long long total = totalBetween(fromDate, toDate);
    SpendingReport report(fromDate, toDate, total, count, totalByCategory(fromDate, toDate), totalByMode(fromDate, toDate));
    return report;
}

}
