#include "DebtCalculator.hpp"

#include <queue>

using namespace std;

namespace expense_tracker
{

// debts[(a, b)] = how much a owes to b (before joining the two directions)
map<pair<int, int>, long long> DebtCalculator::collectDebts(int groupId) const
{
    map<pair<int, int>, long long> debts;

    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, false);
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        int payer = expenses[i].getData().paidBy;
        const vector<SplitShare>& shares = expenses[i].getShares();
        for (int j = 0; j < (int)shares.size(); j++)
        {
            if (shares[j].personId != payer && shares[j].owed > 0)
            {
                debts[make_pair(shares[j].personId, payer)] += shares[j].owed;
            }
        }
    }

    vector<Settlement> payments = split.getSettlements(groupId, false);
    for (int i = 0; i < (int)payments.size(); i++)
    {
        const SettlementData& data = payments[i].getData();
        debts[make_pair(data.fromId, data.toId)] -= data.amount;
    }
    return debts;
}

vector<Debt> DebtCalculator::pairwiseDebts(int groupId) const
{
    map<pair<int, int>, long long> debts = collectDebts(groupId);

    // join the two directions of every pair: positive = the smaller ID owes the bigger ID
    map<pair<int, int>, long long> joined;
    map<pair<int, int>, long long>::iterator it;
    for (it = debts.begin(); it != debts.end(); ++it)
    {
        int from = it->first.first;
        int to = it->first.second;
        if (from < to)
        {
            joined[make_pair(from, to)] += it->second;
        }
        else
        {
            joined[make_pair(to, from)] -= it->second;
        }
    }

    vector<Debt> result;
    for (it = joined.begin(); it != joined.end(); ++it)
    {
        Debt debt;
        if (it->second > 0)
        {
            debt.fromId = it->first.first;
            debt.toId = it->first.second;
            debt.amount = it->second;
            result.push_back(debt);
        }
        else if (it->second < 0)
        {
            debt.fromId = it->first.second;
            debt.toId = it->first.first;
            debt.amount = -it->second;
            result.push_back(debt);
        }
    }
    return result;
}

// positive = this person should get money, negative = this person should pay money
map<int, long long> DebtCalculator::netBalances(int groupId) const
{
    map<int, long long> net;
    vector<Debt> debts = pairwiseDebts(groupId);
    for (int i = 0; i < (int)debts.size(); i++)
    {
        net[debts[i].fromId] -= debts[i].amount;
        net[debts[i].toId] += debts[i].amount;
    }
    return net;
}

// Greedy method with two priority queues: the person who owes the most pays
// the person who should get the most. Every step settles at least one person,
// so there are at most (people - 1) payments.
vector<Debt> DebtCalculator::simplifiedDebts(int groupId) const
{
    map<int, long long> net = netBalances(groupId);

    priority_queue<pair<long long, int> > creditors;
    priority_queue<pair<long long, int> > debtors;
    map<int, long long>::iterator it;
    for (it = net.begin(); it != net.end(); ++it)
    {
        if (it->second > 0)
        {
            creditors.push(make_pair(it->second, -it->first));
        }
        else if (it->second < 0)
        {
            debtors.push(make_pair(-it->second, -it->first));
        }
    }

    vector<Debt> result;
    while (!creditors.empty() && !debtors.empty())
    {
        pair<long long, int> credit = creditors.top();
        pair<long long, int> debt = debtors.top();
        creditors.pop();
        debtors.pop();

        long long pay = credit.first;
        if (debt.first < pay)
        {
            pay = debt.first;
        }

        Debt step;
        step.fromId = -debt.second;
        step.toId = -credit.second;
        step.amount = pay;
        result.push_back(step);

        if (credit.first > pay)
        {
            creditors.push(make_pair(credit.first - pay, credit.second));
        }
        if (debt.first > pay)
        {
            debtors.push(make_pair(debt.first - pay, debt.second));
        }
    }
    return result;
}

// positive = personB owes personA, negative = personA owes personB
long long DebtCalculator::balanceBetween(int personA, int personB, int groupId) const
{
    map<pair<int, int>, long long> debts = collectDebts(groupId);
    long long bOwesA = 0;
    long long aOwesB = 0;

    map<pair<int, int>, long long>::iterator it = debts.find(make_pair(personB, personA));
    if (it != debts.end())
    {
        bOwesA = it->second;
    }
    it = debts.find(make_pair(personA, personB));
    if (it != debts.end())
    {
        aOwesB = it->second;
    }
    return bOwesA - aOwesB;
}

// balance between You (ID 1) and a friend for every group (0 = no group)
map<int, long long> DebtCalculator::balanceByGroupWith(int friendId) const
{
    map<int, long long> result;

    long long noGroup = balanceBetween(1, friendId, 0);
    if (noGroup != 0)
    {
        result[0] = noGroup;
    }

    const vector<Group>& all = groups.getAll();
    for (int i = 0; i < (int)all.size(); i++)
    {
        long long value = balanceBetween(1, friendId, all[i].id);
        if (value != 0)
        {
            result[all[i].id] = value;
        }
    }
    return result;
}

}
