#pragma once

#include <map>
#include <string>
#include <vector>

#include "Budget.hpp"
#include "ExpenseManager.hpp"
#include "SpendingReport.hpp"

namespace expense_tracker
{

struct CategoryTotal
{
    std::string category;
    long long total = 0;
};

struct PeriodComparison
{
    long long firstTotal = 0;
    long long secondTotal = 0;
    long long difference = 0;
    bool hasPercent = false;
    long long percentChange = 0;
    std::string trend;
};

class AnalyticsService
{
public:
    AnalyticsService(const ExpenseManager& manager) : manager(manager)
    {
    }

    long long totalBetween(const Date& fromDate, const Date& toDate) const;
    std::map<std::string, long long> totalByCategory(const Date& fromDate, const Date& toDate) const;
    std::map<PaymentMode, long long> totalByMode(const Date& fromDate, const Date& toDate) const;
    BudgetStatus budgetStatus(const Budget& budget, const Date& fromDate, const Date& toDate) const;

    std::map<std::string, long long> dailySummary(const Date& fromDate, const Date& toDate) const;
    std::map<std::string, long long> weeklySummary(const Date& fromDate, const Date& toDate) const;
    std::map<std::string, long long> monthlySummary(const Date& fromDate, const Date& toDate) const;

    std::vector<CategoryTotal> topCategories(const Date& fromDate, const Date& toDate, int n) const;
    PeriodComparison comparePeriods(const Date& firstFrom, const Date& firstTo,
                                    const Date& secondFrom, const Date& secondTo) const;

    std::vector<long long> makeDailyPrefixSums(const Date& fromDate, const Date& toDate) const;
    long long totalFromPrefix(const std::vector<long long>& prefix, int startDay, int endDay) const;

    SpendingReport makeReport(const Date& fromDate, const Date& toDate) const;

private:
    const ExpenseManager& manager;
};

}
