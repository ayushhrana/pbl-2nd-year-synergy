#pragma once

#include <ostream>
#include <string>

#include "Date.hpp"

namespace expense_tracker
{

enum PaymentMode
{
    CASH,
    ONLINE
};

std::string modeToString(PaymentMode mode);
PaymentMode stringToMode(const std::string& text);

struct ExpenseData
{
    long long amount = 0;
    std::string category;
    Date date;
    std::string note;
    std::string place;
    PaymentMode mode = CASH;
};

class Expense
{
public:
    Expense(int id, const ExpenseData& data);

    int getId() const
    {
        return id;
    }

    long long getAmount() const
    {
        return data.amount;
    }

    std::string getCategory() const
    {
        return data.category;
    }

    Date getDate() const
    {
        return data.date;
    }

    std::string getNote() const
    {
        return data.note;
    }

    std::string getPlace() const
    {
        return data.place;
    }

    PaymentMode getMode() const
    {
        return data.mode;
    }

    ExpenseData getData() const
    {
        return data;
    }

    std::string toString() const;
    bool operator==(const Expense& other) const;

    bool operator!=(const Expense& other) const
    {
        return !(*this == other);
    }

private:
    int id;
    ExpenseData data;
};

bool isAmountLess(const Expense& first, const Expense& second);
bool isDateLess(const Expense& first, const Expense& second);

std::ostream& operator<<(std::ostream& out, const Expense& expense);

}
