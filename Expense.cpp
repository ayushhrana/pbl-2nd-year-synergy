#include "Expense.hpp"

#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

string modeToString(PaymentMode mode)
{
    if (mode == CASH)
    {
        return "cash";
    }
    return "online";
}

PaymentMode stringToMode(const string& text)
{
    string cleanText = toLowerCase(trimSpaces(text));
    if (cleanText == "cash")
    {
        return CASH;
    }
    if (cleanText == "online")
    {
        return ONLINE;
    }
    throw ExpenseError("Payment mode must be cash or online: " + text);
}

Expense::Expense(int id, const ExpenseData& data)
{
    if (id <= 0)
    {
        throw ExpenseError("Expense ID must be greater than zero");
    }
    this->id = id;
    this->data = data;
}

string Expense::toString() const
{
    string text = "#" + to_string(id) + " ";
    text = text + data.date.toString() + " ";
    text = text + paiseToRupees(data.amount) + " ";
    text = text + data.category + " [" + modeToString(data.mode) + "] ";
    text = text + data.note;
    if (!data.place.empty())
    {
        text = text + " @ " + data.place;
    }
    return text;
}

bool Expense::operator==(const Expense& other) const
{
    if (id != other.id)
    {
        return false;
    }
    if (data.amount != other.data.amount)
    {
        return false;
    }
    if (data.category != other.data.category)
    {
        return false;
    }
    if (data.date != other.data.date)
    {
        return false;
    }
    if (data.note != other.data.note)
    {
        return false;
    }
    if (data.place != other.data.place)
    {
        return false;
    }
    if (data.mode != other.data.mode)
    {
        return false;
    }
    return true;
}

bool isAmountLess(const Expense& first, const Expense& second)
{
    if (first.getAmount() != second.getAmount())
    {
        return first.getAmount() < second.getAmount();
    }
    return first.getId() < second.getId();
}

bool isDateLess(const Expense& first, const Expense& second)
{
    if (first.getDate() != second.getDate())
    {
        return first.getDate() < second.getDate();
    }
    return first.getId() < second.getId();
}

ostream& operator<<(ostream& out, const Expense& expense)
{
    out << expense.toString();
    return out;
}

}
