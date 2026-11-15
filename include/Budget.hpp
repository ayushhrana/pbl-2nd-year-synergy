#pragma once

#include <string>

namespace expense_tracker
{

class Budget
{
public:
    Budget(long long limit, const std::string& category);

    long long getLimit() const
    {
        return limit;
    }

    std::string getCategory() const
    {
        return category;
    }

private:
    long long limit;
    std::string category;
};

struct BudgetStatus
{
    long long limit = 0;
    long long spent = 0;
    long long remaining = 0;
    bool exceeded = false;
};

}
