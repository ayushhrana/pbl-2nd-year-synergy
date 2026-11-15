#include "SplitManager.hpp"

#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

SplitManager::SplitManager(const PeopleManager& people, const GroupManager& groups, const CategoryList& categories)
    : people(people), groups(groups), categories(categories)
{
    nextExpenseId = 1;
    nextSettlementId = 1;
}

void SplitManager::checkPerson(int personId, int groupId) const
{
    const Person* person = people.findPerson(personId);
    if (person == nullptr)
    {
        throw ExpenseError("No person found with ID " + to_string(personId));
    }
    if (groupId > 0 && !groups.isMember(groupId, personId))
    {
        throw ExpenseError(person->name + " is not a member of this group");
    }
}

SharedExpenseData SplitManager::cleanData(const SharedExpenseData& data) const
{
    SharedExpenseData result = data;

    result.description = trimSpaces(data.description);
    if (result.description.empty())
    {
        throw ExpenseError("Description cannot be empty");
    }
    if (result.description.size() > 100)
    {
        throw ExpenseError("Description is too long (maximum 100 letters)");
    }

    result.note = trimSpaces(data.note);
    if (result.note.size() > 200)
    {
        throw ExpenseError("Note is too long (maximum 200 letters)");
    }

    result.category = categories.findCategory(data.category);
    checkAmount(data.amount);

    if (data.groupId < 0)
    {
        throw ExpenseError("Wrong group");
    }
    if (data.groupId > 0 && groups.findGroup(data.groupId) == nullptr)
    {
        throw ExpenseError("No group found with ID " + to_string(data.groupId));
    }

    checkPerson(data.paidBy, data.groupId);
    for (int i = 0; i < (int)data.inputs.size(); i++)
    {
        checkPerson(data.inputs[i].personId, data.groupId);
    }
    return result;
}

int SplitManager::addSharedExpense(const SharedExpenseData& data)
{
    SharedExpenseData goodData = cleanData(data);
    vector<SplitShare> shares = calculateSplit(goodData.amount, goodData.splitType, goodData.inputs);

    int id = nextExpenseId;
    sharedExpenses.push_back(SharedExpense(id, goodData, shares));
    nextExpenseId = nextExpenseId + 1;
    return id;
}

void SplitManager::updateSharedExpense(int id, const SharedExpenseData& data)
{
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (sharedExpenses[i].getId() == id)
        {
            SharedExpenseData goodData = cleanData(data);
            vector<SplitShare> shares = calculateSplit(goodData.amount, goodData.splitType, goodData.inputs);
            bool wasDeleted = sharedExpenses[i].isDeleted();
            sharedExpenses[i] = SharedExpense(id, goodData, shares);
            sharedExpenses[i].setDeleted(wasDeleted);
            return;
        }
    }
    throw ExpenseError("No shared expense found with ID " + to_string(id));
}

void SplitManager::deleteSharedExpense(int id)
{
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (sharedExpenses[i].getId() == id)
        {
            if (sharedExpenses[i].isDeleted())
            {
                throw ExpenseError("This expense is already deleted");
            }
            sharedExpenses[i].setDeleted(true);
            return;
        }
    }
    throw ExpenseError("No shared expense found with ID " + to_string(id));
}

void SplitManager::restoreSharedExpense(int id)
{
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (sharedExpenses[i].getId() == id)
        {
            if (!sharedExpenses[i].isDeleted())
            {
                throw ExpenseError("This expense is not deleted");
            }
            sharedExpenses[i].setDeleted(false);
            return;
        }
    }
    throw ExpenseError("No shared expense found with ID " + to_string(id));
}

const SharedExpense* SplitManager::findSharedExpense(int id) const
{
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (sharedExpenses[i].getId() == id)
        {
            return &sharedExpenses[i];
        }
    }
    return nullptr;
}

// groupId: -1 = all expenses, 0 = expenses without a group, bigger than 0 = one group
vector<SharedExpense> SplitManager::getSharedExpenses(int groupId, bool includeDeleted) const
{
    vector<SharedExpense> result;
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (groupId >= 0 && sharedExpenses[i].getData().groupId != groupId)
        {
            continue;
        }
        if (!includeDeleted && sharedExpenses[i].isDeleted())
        {
            continue;
        }
        result.push_back(sharedExpenses[i]);
    }
    return result;
}

int SplitManager::addSettlement(const SettlementData& data)
{
    if (data.fromId == data.toId)
    {
        throw ExpenseError("A person cannot pay himself");
    }
    if (data.groupId < 0)
    {
        throw ExpenseError("Wrong group");
    }
    if (data.groupId > 0 && groups.findGroup(data.groupId) == nullptr)
    {
        throw ExpenseError("No group found with ID " + to_string(data.groupId));
    }
    checkPerson(data.fromId, data.groupId);
    checkPerson(data.toId, data.groupId);
    checkAmount(data.amount);

    SettlementData goodData = data;
    goodData.note = trimSpaces(data.note);
    if (goodData.note.size() > 200)
    {
        throw ExpenseError("Note is too long (maximum 200 letters)");
    }

    int id = nextSettlementId;
    settlements.push_back(Settlement(id, goodData));
    nextSettlementId = nextSettlementId + 1;
    return id;
}

void SplitManager::deleteSettlement(int id)
{
    for (int i = 0; i < (int)settlements.size(); i++)
    {
        if (settlements[i].getId() == id)
        {
            if (settlements[i].isDeleted())
            {
                throw ExpenseError("This payment is already deleted");
            }
            settlements[i].setDeleted(true);
            return;
        }
    }
    throw ExpenseError("No payment found with ID " + to_string(id));
}

const Settlement* SplitManager::findSettlement(int id) const
{
    for (int i = 0; i < (int)settlements.size(); i++)
    {
        if (settlements[i].getId() == id)
        {
            return &settlements[i];
        }
    }
    return nullptr;
}

vector<Settlement> SplitManager::getSettlements(int groupId, bool includeDeleted) const
{
    vector<Settlement> result;
    for (int i = 0; i < (int)settlements.size(); i++)
    {
        if (groupId >= 0 && settlements[i].getData().groupId != groupId)
        {
            continue;
        }
        if (!includeDeleted && settlements[i].isDeleted())
        {
            continue;
        }
        result.push_back(settlements[i]);
    }
    return result;
}

void SplitManager::removeGroupData(int groupId)
{
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (sharedExpenses[i].getData().groupId == groupId)
        {
            sharedExpenses.erase(sharedExpenses.begin() + i);
            i--;
        }
    }
    for (int i = 0; i < (int)settlements.size(); i++)
    {
        if (settlements[i].getData().groupId == groupId)
        {
            settlements.erase(settlements.begin() + i);
            i--;
        }
    }
}

bool SplitManager::isPersonUsed(int personId) const
{
    for (int i = 0; i < (int)sharedExpenses.size(); i++)
    {
        if (sharedExpenses[i].getData().paidBy == personId)
        {
            return true;
        }
        for (int j = 0; j < (int)sharedExpenses[i].getShares().size(); j++)
        {
            if (sharedExpenses[i].getShares()[j].personId == personId)
            {
                return true;
            }
        }
    }
    for (int i = 0; i < (int)settlements.size(); i++)
    {
        if (settlements[i].getData().fromId == personId || settlements[i].getData().toId == personId)
        {
            return true;
        }
    }
    return false;
}

void SplitManager::loadSharedExpense(const SharedExpense& expense)
{
    sharedExpenses.push_back(expense);
}

void SplitManager::loadSettlement(const Settlement& settlement)
{
    settlements.push_back(settlement);
}

void SplitManager::clearAll()
{
    sharedExpenses.clear();
    settlements.clear();
}

}
