#include "FilterCriteria.hpp"

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

void FilterCriteria::check() const
{
    if (useDates && from > to)
    {
        throw ExpenseError("Filter start date is after the end date");
    }
    if (useMin && useMax && minAmount > maxAmount)
    {
        throw ExpenseError("Filter minimum amount is more than maximum amount");
    }
}

// an expense must match every filter that is switched on
bool FilterCriteria::matches(const Expense& expense) const
{
    if (useCategory && !isSameText(expense.getCategory(), category))
    {
        return false;
    }
    if (useDates && (expense.getDate() < from || expense.getDate() > to))
    {
        return false;
    }
    if (useMode && expense.getMode() != mode)
    {
        return false;
    }
    if (useMin && expense.getAmount() < minAmount)
    {
        return false;
    }
    if (useMax && expense.getAmount() > maxAmount)
    {
        return false;
    }
    if (usePlace && !containsText(expense.getPlace(), place))
    {
        return false;
    }
    return true;
}

}
