#pragma once

#include <map>
#include <string>

#include "SplitManager.hpp"

namespace expense_tracker
{

class SplitAnalytics
{
public:
    SplitAnalytics(const SplitManager& split) : split(split)
    {
    }

    long long totalSpent(int groupId) const;
    long long totalPaidBy(int personId, int groupId) const;
    long long totalShareOf(int personId, int groupId) const;
    std::map<std::string, long long> shareByCategory(int personId, int groupId) const;

private:
    const SplitManager& split;
};

}
