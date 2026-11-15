#include "ApiHandler.hpp"

#include "ApiJson.hpp"
#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

JsonValue ApiHandler::listExpenses(const JsonValue& body)
{
    string sortBy = body.getStringOr("sortBy", "id");
    bool ascending = body.getStringOr("order", "asc") != "desc";

    if (sortBy == "amount")
    {
        return expensesToJson(personal.sortByAmount(ascending));
    }
    if (sortBy == "date")
    {
        return expensesToJson(personal.sortByDate(ascending));
    }
    return expensesToJson(personal.sortById(ascending));
}

JsonValue ApiHandler::addExpense(const JsonValue& body)
{
    ExpenseData data = readExpenseData(body, true);
    bool force = body.getBoolOr("force", false);

    JsonValue result = JsonValue::makeObject();
    if (!force && personal.isDuplicate(data))
    {
        result.setBool("added", false);
        result.setBool("duplicate", true);
        return result;
    }

    int id = undoRedo.addExpense(data);
    changed = true;
    const Expense* saved = personal.findExpenseById(id);
    activity.add("You added expense #" + to_string(id) + " (" + saved->getCategory() + ", " + paiseToRupees(saved->getAmount()) + ")");

    result.setBool("added", true);
    result.setBool("duplicate", false);
    result.setNumber("id", id);
    return result;
}

JsonValue ApiHandler::updateExpense(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    undoRedo.updateExpense(id, readExpenseData(body, true));
    changed = true;
    activity.add("You updated expense #" + to_string(id));
    return JsonValue();
}

JsonValue ApiHandler::deleteExpense(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    undoRedo.deleteExpense(id);
    changed = true;
    activity.add("You deleted expense #" + to_string(id));
    return JsonValue();
}

JsonValue ApiHandler::getExpense(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    const Expense* found = search.findExpenseById(id);
    if (found == nullptr)
    {
        throw ExpenseError("No expense found with ID " + to_string(id));
    }
    return expenseToJson(*found);
}

JsonValue ApiHandler::searchExpenses(const JsonValue& body)
{
    return expensesToJson(search.searchText(body.getString("text")));
}

JsonValue ApiHandler::filterExpenses(const JsonValue& body)
{
    FilterCriteria criteria;

    string category = trimSpaces(body.getStringOr("category", ""));
    if (!category.empty())
    {
        criteria.useCategory = true;
        criteria.category = category;
    }

    string fromText = trimSpaces(body.getStringOr("from", ""));
    string toText = trimSpaces(body.getStringOr("to", ""));
    if (!fromText.empty() || !toText.empty())
    {
        criteria.useDates = true;
        criteria.from = Date(1000, 1, 1);
        criteria.to = Date(9999, 12, 31);
        if (!fromText.empty())
        {
            criteria.from = Date::parse(fromText);
        }
        if (!toText.empty())
        {
            criteria.to = Date::parse(toText);
        }
    }

    string modeText = trimSpaces(body.getStringOr("mode", ""));
    if (!modeText.empty())
    {
        criteria.useMode = true;
        criteria.mode = stringToMode(modeText);
    }

    string minText = trimSpaces(body.getStringOr("min", ""));
    if (!minText.empty())
    {
        criteria.useMin = true;
        criteria.minAmount = parseHundredths(minText);
    }

    string maxText = trimSpaces(body.getStringOr("max", ""));
    if (!maxText.empty())
    {
        criteria.useMax = true;
        criteria.maxAmount = parseHundredths(maxText);
    }

    string placeText = trimSpaces(body.getStringOr("place", ""));
    if (!placeText.empty())
    {
        criteria.usePlace = true;
        criteria.place = placeText;
    }

    return expensesToJson(search.filter(criteria));
}

JsonValue ApiHandler::expenseStats()
{
    JsonValue result = JsonValue::makeObject();
    result.setNumber("count", personal.getCount());
    setMoney(result, "total", personal.getTotalAmount());

    if (personal.getCount() > 0)
    {
        setMoney(result, "average", personal.getAverageAmount());
        result.set("smallest", expenseToJson(personal.getSmallestExpense()));
        result.set("largest", expenseToJson(personal.getLargestExpense()));
    }
    else
    {
        setMoney(result, "average", 0);
        result.set("smallest", JsonValue());
        result.set("largest", JsonValue());
    }

    result.set("duplicates", expensesToJson(personal.getDuplicateExpenses()));
    return result;
}

JsonValue ApiHandler::undoStatus()
{
    JsonValue result = JsonValue::makeObject();
    result.setBool("canUndo", undoRedo.canUndo());
    result.setBool("canRedo", undoRedo.canRedo());
    return result;
}

JsonValue ApiHandler::undoAction()
{
    undoRedo.undo();
    changed = true;
    return undoStatus();
}

JsonValue ApiHandler::redoAction()
{
    undoRedo.redo();
    changed = true;
    return undoStatus();
}

JsonValue ApiHandler::listCategories()
{
    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < personal.getCategories().getCount(); i++)
    {
        list.add(JsonValue::makeString(personal.getCategories().getCategoryAt(i)));
    }
    return list;
}

JsonValue ApiHandler::addCategory(const JsonValue& body)
{
    personal.getCategories().addCategory(body.getString("name"));
    changed = true;
    return listCategories();
}

}
