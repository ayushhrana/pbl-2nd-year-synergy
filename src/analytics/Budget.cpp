#include "Budget.hpp"

#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

Budget::Budget(long long limit, const string& category)
{
    checkAmount(limit);
    this->limit = limit;
    this->category = trimSpaces(category);
}

}
