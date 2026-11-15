#include "SplitAnalytics.hpp"

using namespace std;

namespace expense_tracker
{

long long SplitAnalytics::totalSpent(int groupId) const
{
    long long total = 0;
    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, false);
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        total = total + expenses[i].getData().amount;
    }
    return total;
}

long long SplitAnalytics::totalPaidBy(int personId, int groupId) const
{
    long long total = 0;
    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, false);
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        if (expenses[i].getData().paidBy == personId)
        {
            total = total + expenses[i].getData().amount;
        }
    }
    return total;
}

long long SplitAnalytics::totalShareOf(int personId, int groupId) const
{
    long long total = 0;
    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, false);
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        const vector<SplitShare>& shares = expenses[i].getShares();
        for (int j = 0; j < (int)shares.size(); j++)
        {
            if (shares[j].personId == personId)
            {
                total = total + shares[j].owed;
            }
        }
    }
    return total;
}

map<string, long long> SplitAnalytics::shareByCategory(int personId, int groupId) const
{
    map<string, long long> totals;
    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, false);
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        const vector<SplitShare>& shares = expenses[i].getShares();
        for (int j = 0; j < (int)shares.size(); j++)
        {
            if (shares[j].personId == personId && shares[j].owed > 0)
            {
                string category = expenses[i].getData().category;
                totals[category] = totals[category] + shares[j].owed;
            }
        }
    }
    return totals;
}

}
