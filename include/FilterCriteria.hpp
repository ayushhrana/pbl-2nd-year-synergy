#pragma once

#include <string>

#include "Expense.hpp"

namespace expense_tracker
{

struct FilterCriteria
{
    bool useCategory;
    std::string category;
    bool useDates;
    Date from;
    Date to;
    bool useMode;
    PaymentMode mode;
    bool useMin;
    long long minAmount;
    bool useMax;
    long long maxAmount;
    bool usePlace;
    std::string place;

    FilterCriteria()
    {
        useCategory = false;
        useDates = false;
        useMode = false;
        mode = CASH;
        useMin = false;
        minAmount = 0;
        useMax = false;
        maxAmount = 0;
        usePlace = false;
    }

    void check() const;
    bool matches(const Expense& expense) const;
};

}
