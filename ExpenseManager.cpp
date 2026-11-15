#include "ExpenseManager.hpp"

#include <algorithm>

#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

ExpenseManager::ExpenseManager()
{
    nextId = 1;
    version = 0;
}

ExpenseData ExpenseManager::cleanData(const ExpenseData& data) const
{
    checkAmount(data.amount);

    ExpenseData result = data;
    result.category = categories.findCategory(data.category);
    result.note = trimSpaces(data.note);
    result.place = trimSpaces(data.place);
    return result;
}

// binary search on the vector, it works because the vector is always sorted by ID
// gives the position of the ID, or the position where it should be inserted
int ExpenseManager::findPosition(int id) const
{
    int low = 0;
    int high = (int)expenses.size();

    while (low < high)
    {
        int middle = (low + high) / 2;
        if (expenses[middle].getId() < id)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

int ExpenseManager::addExpense(const ExpenseData& data)
{
    ExpenseData goodData = cleanData(data);

    int id = nextId;
    Expense newExpense(id, goodData);
    expenses.push_back(newExpense);

    nextId = nextId + 1;
    version = version + 1;
    return id;
}

void ExpenseManager::updateExpense(int id, const ExpenseData& data)
{
    int position = findPosition(id);
    if (position >= (int)expenses.size() || expenses[position].getId() != id)
    {
        throw ExpenseError("No expense found with ID " + to_string(id));
    }

    ExpenseData goodData = cleanData(data);
    expenses[position] = Expense(id, goodData);
    version = version + 1;
}

Expense ExpenseManager::deleteExpense(int id)
{
    int position = findPosition(id);
    if (position >= (int)expenses.size() || expenses[position].getId() != id)
    {
        throw ExpenseError("No expense found with ID " + to_string(id));
    }

    Expense deleted = expenses[position];
    expenses.erase(expenses.begin() + position);
    version = version + 1;
    return deleted;
}

// used by undo: puts a deleted expense back with the same ID
// nextId is moved forward so this ID is never given to a new expense
void ExpenseManager::restoreExpense(const Expense& expense)
{
    int position = findPosition(expense.getId());
    if (position < (int)expenses.size() && expenses[position].getId() == expense.getId())
    {
        throw ExpenseError("An expense with ID " + to_string(expense.getId()) + " already exists");
    }

    expenses.insert(expenses.begin() + position, expense);

    if (expense.getId() >= nextId)
    {
        nextId = expense.getId() + 1;
    }
    version = version + 1;
}

const Expense* ExpenseManager::findExpenseById(int id) const
{
    int position = findPosition(id);
    if (position < (int)expenses.size() && expenses[position].getId() == id)
    {
        return &expenses[position];
    }
    return nullptr;
}

vector<Expense> ExpenseManager::getExpensesBetween(const Date& fromDate, const Date& toDate) const
{
    if (fromDate > toDate)
    {
        throw ExpenseError("Start date is after the end date");
    }

    vector<Expense> result;
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        Date expenseDate = expenses[i].getDate();
        if (expenseDate >= fromDate && expenseDate <= toDate)
        {
            result.push_back(expenses[i]);
        }
    }
    return result;
}

vector<Expense> ExpenseManager::sortByAmount(bool ascending) const
{
    vector<Expense> copy = expenses;
    sort(copy.begin(), copy.end(), isAmountLess);
    if (!ascending)
    {
        reverse(copy.begin(), copy.end());
    }
    return copy;
}

vector<Expense> ExpenseManager::sortByDate(bool ascending) const
{
    vector<Expense> copy = expenses;
    sort(copy.begin(), copy.end(), isDateLess);
    if (!ascending)
    {
        reverse(copy.begin(), copy.end());
    }
    return copy;
}

vector<Expense> ExpenseManager::sortById(bool ascending) const
{
    vector<Expense> copy = expenses;
    if (!ascending)
    {
        reverse(copy.begin(), copy.end());
    }
    return copy;
}

bool ExpenseManager::isDuplicate(const ExpenseData& data) const
{
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        if (expenses[i].getAmount() != data.amount)
        {
            continue;
        }
        if (!isSameText(expenses[i].getCategory(), trimSpaces(data.category)))
        {
            continue;
        }
        if (expenses[i].getDate() != data.date)
        {
            continue;
        }
        if (expenses[i].getMode() != data.mode)
        {
            continue;
        }
        if (!isSameText(expenses[i].getNote(), trimSpaces(data.note)))
        {
            continue;
        }
        return true;
    }
    return false;
}

vector<Expense> ExpenseManager::getDuplicateExpenses() const
{
    vector<bool> hasTwin(expenses.size(), false);

    for (int i = 0; i < (int)expenses.size(); i++)
    {
        for (int j = i + 1; j < (int)expenses.size(); j++)
        {
            bool same = true;
            if (expenses[i].getAmount() != expenses[j].getAmount())
            {
                same = false;
            }
            if (expenses[i].getCategory() != expenses[j].getCategory())
            {
                same = false;
            }
            if (expenses[i].getDate() != expenses[j].getDate())
            {
                same = false;
            }
            if (expenses[i].getMode() != expenses[j].getMode())
            {
                same = false;
            }
            if (!isSameText(expenses[i].getNote(), expenses[j].getNote()))
            {
                same = false;
            }
            if (same)
            {
                hasTwin[i] = true;
                hasTwin[j] = true;
            }
        }
    }

    vector<Expense> result;
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        if (hasTwin[i])
        {
            result.push_back(expenses[i]);
        }
    }
    return result;
}

long long ExpenseManager::getTotalAmount() const
{
    long long total = 0;
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        total = total + expenses[i].getAmount();
    }
    return total;
}

long long ExpenseManager::getAverageAmount() const
{
    if (expenses.empty())
    {
        throw ExpenseError("There are no expenses");
    }
    return getTotalAmount() / (long long)expenses.size();
}

Expense ExpenseManager::getSmallestExpense() const
{
    if (expenses.empty())
    {
        throw ExpenseError("There are no expenses");
    }
    int smallest = 0;
    for (int i = 1; i < (int)expenses.size(); i++)
    {
        if (expenses[i].getAmount() < expenses[smallest].getAmount())
        {
            smallest = i;
        }
    }
    return expenses[smallest];
}

Expense ExpenseManager::getLargestExpense() const
{
    if (expenses.empty())
    {
        throw ExpenseError("There are no expenses");
    }
    int largest = 0;
    for (int i = 1; i < (int)expenses.size(); i++)
    {
        if (expenses[i].getAmount() > expenses[largest].getAmount())
        {
            largest = i;
        }
    }
    return expenses[largest];
}

}
