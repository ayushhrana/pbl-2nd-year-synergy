#pragma once

#include <vector>

#include "Category.hpp"
#include "Expense.hpp"

namespace expense_tracker
{

class ExpenseManager
{
public:
    ExpenseManager();

    int addExpense(const ExpenseData& data);
    void updateExpense(int id, const ExpenseData& data);
    Expense deleteExpense(int id);
    void restoreExpense(const Expense& expense);

    const Expense* findExpenseById(int id) const;
    std::vector<Expense> getExpensesBetween(const Date& fromDate, const Date& toDate) const;

    std::vector<Expense> sortByAmount(bool ascending) const;
    std::vector<Expense> sortByDate(bool ascending) const;
    std::vector<Expense> sortById(bool ascending) const;

    bool isDuplicate(const ExpenseData& data) const;
    std::vector<Expense> getDuplicateExpenses() const;

    long long getTotalAmount() const;
    long long getAverageAmount() const;
    Expense getSmallestExpense() const;
    Expense getLargestExpense() const;

    int getNextId() const
    {
        return nextId;
    }

    void setNextId(int newNextId)
    {
        nextId = newNextId;
    }

    const std::vector<Expense>& getAllExpenses() const
    {
        return expenses;
    }

    int getCount() const
    {
        return (int)expenses.size();
    }

    int getVersion() const
    {
        return version;
    }

    CategoryList& getCategories()
    {
        return categories;
    }

    const CategoryList& getCategories() const
    {
        return categories;
    }

private:
    ExpenseData cleanData(const ExpenseData& data) const;
    int findPosition(int id) const;

    std::vector<Expense> expenses;
    CategoryList categories;
    int nextId;
    int version;
};

}
