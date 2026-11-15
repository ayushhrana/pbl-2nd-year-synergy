#pragma once

#include <stdexcept>
#include <string>

namespace expense_tracker
{

class ExpenseError : public std::runtime_error
{
public:
    ExpenseError(const std::string& message) : std::runtime_error(message)
    {
    }
};

}
