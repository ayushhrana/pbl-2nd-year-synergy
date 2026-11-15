#include "Category.hpp"

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

CategoryList::CategoryList()
{
    names.push_back("Food");
    names.push_back("Transport");
    names.push_back("Shopping");
    names.push_back("Bills");
    names.push_back("Entertainment");
    names.push_back("Health");
    names.push_back("Education");
    names.push_back("Other");
}

bool CategoryList::hasCategory(const string& name) const
{
    string cleanName = trimSpaces(name);
    for (int i = 0; i < (int)names.size(); i++)
    {
        if (isSameText(names[i], cleanName))
        {
            return true;
        }
    }
    return false;
}

string CategoryList::findCategory(const string& name) const
{
    string cleanName = trimSpaces(name);
    for (int i = 0; i < (int)names.size(); i++)
    {
        if (isSameText(names[i], cleanName))
        {
            return names[i];
        }
    }
    throw ExpenseError("Category not found: " + name);
}

void CategoryList::addCategory(const string& name)
{
    string cleanName = trimSpaces(name);
    if (cleanName.empty())
    {
        throw ExpenseError("Category name cannot be empty");
    }
    if (hasCategory(cleanName))
    {
        throw ExpenseError("Category already exists: " + cleanName);
    }
    names.push_back(cleanName);
}

string CategoryList::getCategoryAt(int position) const
{
    if (position < 0 || position >= (int)names.size())
    {
        throw ExpenseError("Category position is wrong");
    }
    return names[position];
}

}
