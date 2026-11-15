#include "ExpenseScheduler.hpp"

#include "Errors.hpp"

using namespace std;

namespace expense_tracker
{

// makes the priority queue give the earliest date first
bool ExpenseScheduler::LaterFirst::operator()(const Occurrence& first, const Occurrence& second) const
{
    if (first.due != second.due)
    {
        return first.due > second.due;
    }
    if (first.ruleId != second.ruleId)
    {
        return first.ruleId > second.ruleId;
    }
    return first.index > second.index;
}

void ExpenseScheduler::scheduleOccurrence(int ruleId, int index)
{
    try
    {
        Date due = rules[ruleId - 1].getOccurrenceDate(index);
        if (!rules[ruleId - 1].isAfterEnd(due))
        {
            Occurrence next;
            next.ruleId = ruleId;
            next.index = index;
            next.due = due;
            queue.push(next);
        }
    }
    catch (const ExpenseError&)
    {
        // the date went after year 9999, so there are no more occurrences
    }
}

int ExpenseScheduler::addRule(const RecurringExpense& rule)
{
    return addRuleWithProgress(rule, 0);
}

int ExpenseScheduler::addRuleWithProgress(const RecurringExpense& rule, int takenCount)
{
    rules.push_back(rule);
    takenCounts.push_back(takenCount);
    int ruleId = (int)rules.size();
    scheduleOccurrence(ruleId, takenCount);
    return ruleId;
}

int ExpenseScheduler::getTakenCount(int ruleId) const
{
    if (ruleId < 1 || ruleId > (int)rules.size())
    {
        throw ExpenseError("No recurring rule with ID " + to_string(ruleId));
    }
    return takenCounts[ruleId - 1];
}

const RecurringExpense& ExpenseScheduler::getRule(int ruleId) const
{
    if (ruleId < 1 || ruleId > (int)rules.size())
    {
        throw ExpenseError("No recurring rule with ID " + to_string(ruleId));
    }
    return rules[ruleId - 1];
}

bool ExpenseScheduler::getNextOccurrence(Occurrence& result) const
{
    if (queue.empty())
    {
        return false;
    }
    result = queue.top();
    return true;
}

// every rule keeps only one occurrence in the queue at a time.
// when it is taken out, the next one of that rule is added,
// so the same occurrence can never come out twice
vector<Occurrence> ExpenseScheduler::takeDueOccurrences(const Date& today)
{
    vector<Occurrence> dueList;
    while (!queue.empty() && queue.top().due <= today)
    {
        Occurrence next = queue.top();
        queue.pop();
        dueList.push_back(next);
        takenCounts[next.ruleId - 1] = next.index + 1;
        scheduleOccurrence(next.ruleId, next.index + 1);
    }
    return dueList;
}

}
