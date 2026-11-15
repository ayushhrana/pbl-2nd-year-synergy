#pragma once

#include <string>
#include <vector>

#include "Date.hpp"

namespace expense_tracker
{

enum SplitType
{
    SPLIT_EQUAL,
    SPLIT_EXACT,
    SPLIT_PERCENT,
    SPLIT_SHARES
};

std::string splitTypeToString(SplitType type);
SplitType stringToSplitType(const std::string& text);

struct SplitInput
{
    int personId = 0;
    long long value = 0;
};

struct SplitShare
{
    int personId = 0;
    long long inputValue = 0;
    long long owed = 0;
};

struct SharedExpenseData
{
    int groupId = 0;
    std::string description;
    std::string category;
    long long amount = 0;
    Date date;
    int paidBy = 0;
    SplitType splitType = SPLIT_EQUAL;
    std::vector<SplitInput> inputs;
    std::string note;
};

std::vector<SplitShare> calculateSplit(long long total, SplitType type, const std::vector<SplitInput>& inputs);

class SharedExpense
{
public:
    SharedExpense(int id, const SharedExpenseData& data, const std::vector<SplitShare>& shares);

    int getId() const
    {
        return id;
    }

    const SharedExpenseData& getData() const
    {
        return data;
    }

    const std::vector<SplitShare>& getShares() const
    {
        return shares;
    }

    bool isDeleted() const
    {
        return deleted;
    }

    void setDeleted(bool value)
    {
        deleted = value;
    }

private:
    int id;
    SharedExpenseData data;
    std::vector<SplitShare> shares;
    bool deleted;
};

}
