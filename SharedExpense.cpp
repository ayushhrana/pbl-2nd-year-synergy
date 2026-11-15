#include "SharedExpense.hpp"

#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

string splitTypeToString(SplitType type)
{
    if (type == SPLIT_EQUAL)
    {
        return "equal";
    }
    if (type == SPLIT_EXACT)
    {
        return "exact";
    }
    if (type == SPLIT_PERCENT)
    {
        return "percent";
    }
    return "shares";
}

SplitType stringToSplitType(const string& text)
{
    string cleanText = toLowerCase(trimSpaces(text));
    if (cleanText == "equal")
    {
        return SPLIT_EQUAL;
    }
    if (cleanText == "exact")
    {
        return SPLIT_EXACT;
    }
    if (cleanText == "percent")
    {
        return SPLIT_PERCENT;
    }
    if (cleanText == "shares")
    {
        return SPLIT_SHARES;
    }
    throw ExpenseError("Split type must be equal, exact, percent or shares: " + text);
}

SharedExpense::SharedExpense(int id, const SharedExpenseData& data, const vector<SplitShare>& shares)
{
    this->id = id;
    this->data = data;
    this->shares = shares;
    deleted = false;
}

// the leftover paise (because of rounding) are given one by one to the first people
static void shareLeftover(vector<SplitShare>& result, long long total)
{
    long long given = 0;
    for (int i = 0; i < (int)result.size(); i++)
    {
        given = given + result[i].owed;
    }
    long long leftover = total - given;

    for (int i = 0; leftover > 0 && i < (int)result.size(); i++)
    {
        result[i].owed = result[i].owed + 1;
        leftover--;
    }
}

vector<SplitShare> calculateSplit(long long total, SplitType type, const vector<SplitInput>& inputs)
{
    if (inputs.empty())
    {
        throw ExpenseError("Choose at least one person to split with");
    }

    vector<SplitShare> result;
    for (int i = 0; i < (int)inputs.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (inputs[j].personId == inputs[i].personId)
            {
                throw ExpenseError("A person is selected more than once");
            }
        }
        SplitShare share;
        share.personId = inputs[i].personId;
        share.inputValue = inputs[i].value;
        result.push_back(share);
    }

    int count = (int)result.size();

    if (type == SPLIT_EQUAL)
    {
        long long base = total / count;
        long long extra = total % count;
        for (int i = 0; i < count; i++)
        {
            result[i].inputValue = 0;
            result[i].owed = base;
            if (i < extra)
            {
                result[i].owed = result[i].owed + 1;
            }
        }
        return result;
    }

    if (type == SPLIT_EXACT)
    {
        long long sum = 0;
        for (int i = 0; i < count; i++)
        {
            if (inputs[i].value < 0)
            {
                throw ExpenseError("An exact amount cannot be negative");
            }
            result[i].owed = inputs[i].value;
            sum = sum + inputs[i].value;
        }
        if (sum != total)
        {
            throw ExpenseError("The exact amounts add up to " + paiseToRupees(sum) + " but the total is " + paiseToRupees(total));
        }
        return result;
    }

    if (type == SPLIT_PERCENT)
    {
        long long sum = 0;
        for (int i = 0; i < count; i++)
        {
            if (inputs[i].value < 0)
            {
                throw ExpenseError("A percentage cannot be negative");
            }
            sum = sum + inputs[i].value;
        }
        if (sum != 10000)
        {
            throw ExpenseError("The percentages must add up to 100");
        }
        for (int i = 0; i < count; i++)
        {
            result[i].owed = total * inputs[i].value / 10000;
        }
        shareLeftover(result, total);
        return result;
    }

    long long totalShares = 0;
    for (int i = 0; i < count; i++)
    {
        if (inputs[i].value <= 0)
        {
            throw ExpenseError("Every share must be at least 1");
        }
        totalShares = totalShares + inputs[i].value;
    }
    for (int i = 0; i < count; i++)
    {
        result[i].owed = total * inputs[i].value / totalShares;
    }
    shareLeftover(result, total);
    return result;
}

}
