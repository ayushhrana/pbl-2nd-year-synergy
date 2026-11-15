#pragma once

#include <queue>
#include <vector>

#include "RecurringExpense.hpp"

namespace expense_tracker
{

struct Occurrence
{
    int ruleId = 0;
    int index = 0;
    Date due;
};

class ExpenseScheduler
{
public:
    int addRule(const RecurringExpense& rule);
    int addRuleWithProgress(const RecurringExpense& rule, int takenCount);
    int getTakenCount(int ruleId) const;
    const RecurringExpense& getRule(int ruleId) const;
    bool getNextOccurrence(Occurrence& result) const;
    std::vector<Occurrence> takeDueOccurrences(const Date& today);

    int getRuleCount() const
    {
        return (int)rules.size();
    }

    int getPendingCount() const
    {
        return (int)queue.size();
    }

private:
    struct LaterFirst
    {
        bool operator()(const Occurrence& first, const Occurrence& second) const;
    };

    void scheduleOccurrence(int ruleId, int index);

    std::priority_queue<Occurrence, std::vector<Occurrence>, LaterFirst> queue;
    std::vector<RecurringExpense> rules;
    std::vector<int> takenCounts;
};

}
