#include "SearchEngine.hpp"

#include <algorithm>

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

vector<Expense> SearchEngine::filter(const FilterCriteria& criteria) const
{
    criteria.check();

    vector<Expense> result;
    const vector<Expense>& all = manager.getAllExpenses();
    for (int i = 0; i < (int)all.size(); i++)
    {
        if (criteria.matches(all[i]))
        {
            result.push_back(all[i]);
        }
    }
    return result;
}

vector<Expense> SearchEngine::searchText(const string& text) const
{
    string word = trimSpaces(text);
    if (word.empty())
    {
        throw ExpenseError("Search text is empty");
    }

    vector<Expense> result;
    const vector<Expense>& all = manager.getAllExpenses();
    for (int i = 0; i < (int)all.size(); i++)
    {
        if (containsText(all[i].getNote(), word) || containsText(all[i].getPlace(), word))
        {
            result.push_back(all[i]);
        }
    }
    return result;
}

// the hash map is built again only when the manager has changed (version is different)
void SearchEngine::updateIndex()
{
    if (indexVersion == manager.getVersion())
    {
        return;
    }

    index.clear();
    const vector<Expense>& all = manager.getAllExpenses();
    for (int i = 0; i < (int)all.size(); i++)
    {
        index[all[i].getId()] = i;
    }
    indexVersion = manager.getVersion();
}

const Expense* SearchEngine::findExpenseById(int id)
{
    updateIndex();

    unordered_map<int, int>::iterator it = index.find(id);
    if (it == index.end())
    {
        return nullptr;
    }
    return &manager.getAllExpenses()[it->second];
}

vector<Expense> SearchEngine::sortResultsByAmount(vector<Expense> items, bool ascending)
{
    sort(items.begin(), items.end(), isAmountLess);
    if (!ascending)
    {
        reverse(items.begin(), items.end());
    }
    return items;
}

vector<Expense> SearchEngine::sortResultsByDate(vector<Expense> items, bool ascending)
{
    sort(items.begin(), items.end(), isDateLess);
    if (!ascending)
    {
        reverse(items.begin(), items.end());
    }
    return items;
}

}
