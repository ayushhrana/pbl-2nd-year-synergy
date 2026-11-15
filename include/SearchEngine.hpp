#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ExpenseManager.hpp"
#include "FilterCriteria.hpp"

namespace expense_tracker
{

class SearchEngine
{
public:
    SearchEngine(const ExpenseManager& manager) : manager(manager)
    {
        indexVersion = -1;
    }

    std::vector<Expense> filter(const FilterCriteria& criteria) const;
    std::vector<Expense> searchText(const std::string& text) const;
    const Expense* findExpenseById(int id);

    static std::vector<Expense> sortResultsByAmount(std::vector<Expense> items, bool ascending);
    static std::vector<Expense> sortResultsByDate(std::vector<Expense> items, bool ascending);

private:
    void updateIndex();

    const ExpenseManager& manager;
    std::unordered_map<int, int> index;
    int indexVersion;
};

}
