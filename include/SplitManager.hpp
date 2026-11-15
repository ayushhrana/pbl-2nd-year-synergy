#pragma once

#include <vector>

#include "Category.hpp"
#include "Group.hpp"
#include "People.hpp"
#include "Settlement.hpp"
#include "SharedExpense.hpp"

namespace expense_tracker
{

class SplitManager
{
public:
    SplitManager(const PeopleManager& people, const GroupManager& groups, const CategoryList& categories);

    int addSharedExpense(const SharedExpenseData& data);
    void updateSharedExpense(int id, const SharedExpenseData& data);
    void deleteSharedExpense(int id);
    void restoreSharedExpense(int id);
    const SharedExpense* findSharedExpense(int id) const;
    std::vector<SharedExpense> getSharedExpenses(int groupId, bool includeDeleted) const;

    int addSettlement(const SettlementData& data);
    void deleteSettlement(int id);
    const Settlement* findSettlement(int id) const;
    std::vector<Settlement> getSettlements(int groupId, bool includeDeleted) const;

    void removeGroupData(int groupId);
    bool isPersonUsed(int personId) const;

    void loadSharedExpense(const SharedExpense& expense);
    void loadSettlement(const Settlement& settlement);
    void clearAll();

    const std::vector<SharedExpense>& getAllSharedExpenses() const
    {
        return sharedExpenses;
    }

    const std::vector<Settlement>& getAllSettlements() const
    {
        return settlements;
    }

    int getNextExpenseId() const
    {
        return nextExpenseId;
    }

    void setNextExpenseId(int value)
    {
        nextExpenseId = value;
    }

    int getNextSettlementId() const
    {
        return nextSettlementId;
    }

    void setNextSettlementId(int value)
    {
        nextSettlementId = value;
    }

private:
    SharedExpenseData cleanData(const SharedExpenseData& data) const;
    void checkPerson(int personId, int groupId) const;

    const PeopleManager& people;
    const GroupManager& groups;
    const CategoryList& categories;
    std::vector<SharedExpense> sharedExpenses;
    std::vector<Settlement> settlements;
    int nextExpenseId;
    int nextSettlementId;
};

}
