#pragma once

#include <map>
#include <vector>

#include "Group.hpp"
#include "SplitManager.hpp"

namespace expense_tracker
{

struct Debt
{
    int fromId = 0;
    int toId = 0;
    long long amount = 0;
};

// groupId in all functions: -1 = everything, 0 = expenses without a group, bigger than 0 = one group
class DebtCalculator
{
public:
    DebtCalculator(const SplitManager& split, const GroupManager& groups) : split(split), groups(groups)
    {
    }

    std::map<int, long long> netBalances(int groupId) const;
    std::vector<Debt> pairwiseDebts(int groupId) const;
    std::vector<Debt> simplifiedDebts(int groupId) const;
    long long balanceBetween(int personA, int personB, int groupId) const;
    std::map<int, long long> balanceByGroupWith(int friendId) const;

private:
    std::map<std::pair<int, int>, long long> collectDebts(int groupId) const;

    const SplitManager& split;
    const GroupManager& groups;
};

}
