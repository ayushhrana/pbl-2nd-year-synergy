#include "ApiHandler.hpp"

#include "ApiJson.hpp"
#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

string ApiHandler::groupName(int groupId) const
{
    if (groupId == 0)
    {
        return "Non-group expenses";
    }
    const Group* group = groups.findGroup(groupId);
    if (group == nullptr)
    {
        return "Unknown group";
    }
    return group->name;
}

long long ApiHandler::netOf(int groupId, int personId) const
{
    map<int, long long> net = debts.netBalances(groupId);
    map<int, long long>::iterator it = net.find(personId);
    if (it == net.end())
    {
        return 0;
    }
    return it->second;
}

JsonValue ApiHandler::listPeople()
{
    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < (int)people.getAll().size(); i++)
    {
        list.add(personToJson(people.getAll()[i]));
    }
    return list;
}

JsonValue ApiHandler::addPerson(const JsonValue& body)
{
    int id = people.addPerson(body.getString("name"));
    changed = true;
    activity.add("You added " + people.getName(id) + " as a friend");

    JsonValue result = JsonValue::makeObject();
    result.setNumber("id", id);
    return result;
}

JsonValue ApiHandler::renamePerson(const JsonValue& body)
{
    people.renamePerson((int)body.getNumber("id"), body.getString("name"));
    changed = true;
    return JsonValue();
}

JsonValue ApiHandler::removePerson(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    if (split.isPersonUsed(id))
    {
        throw ExpenseError("This person has expenses or payments, so he or she cannot be removed");
    }
    for (int i = 0; i < (int)groups.getAll().size(); i++)
    {
        if (groups.isMember(groups.getAll()[i].id, id))
        {
            throw ExpenseError("Remove this person from all groups first");
        }
    }
    string name = people.getName(id);
    people.removePerson(id);
    changed = true;
    activity.add("You removed " + name + " from your friends");
    return JsonValue();
}

JsonValue ApiHandler::listGroups()
{
    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < (int)groups.getAll().size(); i++)
    {
        const Group& group = groups.getAll()[i];
        JsonValue item = groupToJson(group, people);
        setMoney(item, "yourBalance", netOf(group.id, 1));
        setMoney(item, "totalSpent", splitAnalytics.totalSpent(group.id));
        list.add(item);
    }
    return list;
}

JsonValue ApiHandler::createGroup(const JsonValue& body)
{
    vector<int> members;
    const JsonValue& memberList = body.get("members");
    for (int i = 0; i < memberList.size(); i++)
    {
        members.push_back((int)memberList.at(i).asNumber());
    }

    int id = groups.createGroup(body.getString("name"), members, people);
    changed = true;
    activity.add("You created the group " + groupName(id));

    JsonValue result = JsonValue::makeObject();
    result.setNumber("id", id);
    return result;
}

JsonValue ApiHandler::addGroupMember(const JsonValue& body)
{
    int groupId = (int)body.getNumber("groupId");
    int personId = (int)body.getNumber("personId");
    groups.addMember(groupId, personId, people);
    changed = true;
    activity.add("You added " + people.getName(personId) + " to " + groupName(groupId));
    return JsonValue();
}

JsonValue ApiHandler::removeGroupMember(const JsonValue& body)
{
    int groupId = (int)body.getNumber("groupId");
    int personId = (int)body.getNumber("personId");
    if (netOf(groupId, personId) != 0)
    {
        throw ExpenseError(people.getName(personId) + " still has a balance in this group. Settle up first");
    }
    groups.removeMember(groupId, personId);
    changed = true;
    activity.add("You removed " + people.getName(personId) + " from " + groupName(groupId));
    return JsonValue();
}

JsonValue ApiHandler::deleteGroup(const JsonValue& body)
{
    int groupId = (int)body.getNumber("groupId");
    if (groups.findGroup(groupId) == nullptr)
    {
        throw ExpenseError("No group found with ID " + to_string(groupId));
    }

    map<int, long long> net = debts.netBalances(groupId);
    map<int, long long>::iterator it;
    for (it = net.begin(); it != net.end(); ++it)
    {
        if (it->second != 0)
        {
            throw ExpenseError("Settle up all balances before deleting the group");
        }
    }

    string name = groupName(groupId);
    split.removeGroupData(groupId);
    groups.deleteGroup(groupId);
    changed = true;
    activity.add("You deleted the group " + name);
    return JsonValue();
}

JsonValue ApiHandler::getGroup(const JsonValue& body)
{
    int groupId = (int)body.getNumber("groupId");
    bool simplify = body.getBoolOr("simplify", true);

    const Group* group = groups.findGroup(groupId);
    if (group == nullptr)
    {
        throw ExpenseError("No group found with ID " + to_string(groupId));
    }

    JsonValue result = JsonValue::makeObject();
    result.set("group", groupToJson(*group, people));

    JsonValue expenseList = JsonValue::makeArray();
    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, true);
    for (int i = (int)expenses.size() - 1; i >= 0; i--)
    {
        expenseList.add(sharedExpenseToJson(expenses[i], people, groups));
    }
    result.set("expenses", expenseList);

    JsonValue paymentList = JsonValue::makeArray();
    vector<Settlement> payments = split.getSettlements(groupId, true);
    for (int i = (int)payments.size() - 1; i >= 0; i--)
    {
        paymentList.add(settlementToJson(payments[i], people));
    }
    result.set("settlements", paymentList);

    map<int, long long> net = debts.netBalances(groupId);
    JsonValue balances = JsonValue::makeArray();
    for (int i = 0; i < (int)group->members.size(); i++)
    {
        JsonValue item = JsonValue::makeObject();
        item.setNumber("id", group->members[i]);
        item.setString("name", people.getName(group->members[i]));
        long long value = 0;
        if (net.find(group->members[i]) != net.end())
        {
            value = net[group->members[i]];
        }
        setMoney(item, "balance", value);
        balances.add(item);
    }
    result.set("balances", balances);

    vector<Debt> debtList;
    if (simplify)
    {
        debtList = debts.simplifiedDebts(groupId);
    }
    else
    {
        debtList = debts.pairwiseDebts(groupId);
    }
    JsonValue debtArray = JsonValue::makeArray();
    for (int i = 0; i < (int)debtList.size(); i++)
    {
        debtArray.add(debtToJson(debtList[i], people));
    }
    result.set("debts", debtArray);
    result.setBool("simplified", simplify);

    setMoney(result, "totalSpent", splitAnalytics.totalSpent(groupId));
    setMoney(result, "paidByYou", splitAnalytics.totalPaidBy(1, groupId));
    setMoney(result, "yourShare", splitAnalytics.totalShareOf(1, groupId));
    return result;
}

JsonValue ApiHandler::addShared(const JsonValue& body)
{
    SharedExpenseData data = readSharedData(body);
    int id = split.addSharedExpense(data);
    changed = true;
    activity.add("You added \"" + split.findSharedExpense(id)->getData().description + "\" (" +
                 paiseToRupees(data.amount) + ") in " + groupName(data.groupId));

    JsonValue result = JsonValue::makeObject();
    result.setNumber("id", id);
    return result;
}

JsonValue ApiHandler::updateShared(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    split.updateSharedExpense(id, readSharedData(body));
    changed = true;
    activity.add("You updated \"" + split.findSharedExpense(id)->getData().description + "\"");
    return JsonValue();
}

JsonValue ApiHandler::deleteShared(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    split.deleteSharedExpense(id);
    changed = true;
    activity.add("You deleted \"" + split.findSharedExpense(id)->getData().description + "\"");
    return JsonValue();
}

JsonValue ApiHandler::restoreShared(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    split.restoreSharedExpense(id);
    changed = true;
    activity.add("You restored \"" + split.findSharedExpense(id)->getData().description + "\"");
    return JsonValue();
}

JsonValue ApiHandler::listShared(const JsonValue& body)
{
    int groupId = (int)body.getNumberOr("groupId", -1);
    bool includeDeleted = body.getBoolOr("includeDeleted", false);

    JsonValue list = JsonValue::makeArray();
    vector<SharedExpense> expenses = split.getSharedExpenses(groupId, includeDeleted);
    for (int i = (int)expenses.size() - 1; i >= 0; i--)
    {
        list.add(sharedExpenseToJson(expenses[i], people, groups));
    }
    return list;
}

JsonValue ApiHandler::addSettlement(const JsonValue& body)
{
    SettlementData data = readSettlementData(body);
    int id = split.addSettlement(data);
    changed = true;
    activity.add(people.getName(data.fromId) + " paid " + people.getName(data.toId) + " " +
                 paiseToRupees(data.amount) + " (" + groupName(data.groupId) + ")");

    JsonValue result = JsonValue::makeObject();
    result.setNumber("id", id);
    return result;
}

JsonValue ApiHandler::deleteSettlement(const JsonValue& body)
{
    int id = (int)body.getNumber("id");
    split.deleteSettlement(id);
    changed = true;
    activity.add("You deleted payment #" + to_string(id));
    return JsonValue();
}

JsonValue ApiHandler::balancesOverview()
{
    long long youAreOwed = 0;
    long long youOwe = 0;

    JsonValue friendList = JsonValue::makeArray();
    for (int i = 0; i < (int)people.getAll().size(); i++)
    {
        const Person& person = people.getAll()[i];
        if (person.id == 1)
        {
            continue;
        }

        long long balance = debts.balanceBetween(1, person.id, -1);
        if (balance > 0)
        {
            youAreOwed = youAreOwed + balance;
        }
        else
        {
            youOwe = youOwe - balance;
        }

        JsonValue item = personToJson(person);
        setMoney(item, "balance", balance);

        JsonValue breakdown = JsonValue::makeArray();
        map<int, long long> byGroup = debts.balanceByGroupWith(person.id);
        map<int, long long>::iterator it;
        for (it = byGroup.begin(); it != byGroup.end(); ++it)
        {
            JsonValue part = JsonValue::makeObject();
            part.setNumber("groupId", it->first);
            part.setString("groupName", groupName(it->first));
            setMoney(part, "balance", it->second);
            breakdown.add(part);
        }
        item.set("groups", breakdown);
        friendList.add(item);
    }

    JsonValue groupList = JsonValue::makeArray();
    for (int i = 0; i < (int)groups.getAll().size(); i++)
    {
        JsonValue item = JsonValue::makeObject();
        item.setNumber("id", groups.getAll()[i].id);
        item.setString("name", groups.getAll()[i].name);
        setMoney(item, "balance", netOf(groups.getAll()[i].id, 1));
        groupList.add(item);
    }

    JsonValue result = JsonValue::makeObject();
    setMoney(result, "youAreOwed", youAreOwed);
    setMoney(result, "youOwe", youOwe);
    setMoney(result, "net", youAreOwed - youOwe);
    result.set("people", friendList);
    result.set("groups", groupList);
    return result;
}

JsonValue ApiHandler::balancesPerson(const JsonValue& body)
{
    int friendId = (int)body.getNumber("personId");
    if (people.findPerson(friendId) == nullptr || friendId == 1)
    {
        throw ExpenseError("No friend found with ID " + to_string(friendId));
    }

    JsonValue result = JsonValue::makeObject();
    result.set("person", personToJson(*people.findPerson(friendId)));
    setMoney(result, "balance", debts.balanceBetween(1, friendId, -1));

    JsonValue breakdown = JsonValue::makeArray();
    map<int, long long> byGroup = debts.balanceByGroupWith(friendId);
    map<int, long long>::iterator it;
    for (it = byGroup.begin(); it != byGroup.end(); ++it)
    {
        JsonValue part = JsonValue::makeObject();
        part.setNumber("groupId", it->first);
        part.setString("groupName", groupName(it->first));
        setMoney(part, "balance", it->second);
        breakdown.add(part);
    }
    result.set("groups", breakdown);

    // every expense and payment between You and this friend (effect > 0 means the friend owes you more)
    JsonValue history = JsonValue::makeArray();
    vector<SharedExpense> expenses = split.getSharedExpenses(-1, false);
    for (int i = (int)expenses.size() - 1; i >= 0; i--)
    {
        const SharedExpenseData& data = expenses[i].getData();
        long long effect = 0;
        bool involved = false;
        for (int j = 0; j < (int)expenses[i].getShares().size(); j++)
        {
            const SplitShare& share = expenses[i].getShares()[j];
            if (data.paidBy == 1 && share.personId == friendId)
            {
                effect = share.owed;
                involved = true;
            }
            if (data.paidBy == friendId && share.personId == 1)
            {
                effect = -share.owed;
                involved = true;
            }
        }
        if (involved)
        {
            JsonValue item = JsonValue::makeObject();
            item.setString("type", "expense");
            item.setNumber("id", expenses[i].getId());
            item.setString("text", data.description);
            item.setString("date", data.date.toString());
            item.setString("groupName", groupName(data.groupId));
            setMoney(item, "amount", data.amount);
            setMoney(item, "effect", effect);
            history.add(item);
        }
    }

    vector<Settlement> payments = split.getSettlements(-1, false);
    for (int i = (int)payments.size() - 1; i >= 0; i--)
    {
        const SettlementData& data = payments[i].getData();
        long long effect = 0;
        if (data.fromId == 1 && data.toId == friendId)
        {
            effect = data.amount;
        }
        else if (data.fromId == friendId && data.toId == 1)
        {
            effect = -data.amount;
        }
        else
        {
            continue;
        }
        JsonValue item = JsonValue::makeObject();
        item.setString("type", "payment");
        item.setNumber("id", payments[i].getId());
        item.setString("text", "Payment");
        item.setString("date", data.date.toString());
        item.setString("groupName", groupName(data.groupId));
        setMoney(item, "amount", data.amount);
        setMoney(item, "effect", effect);
        history.add(item);
    }
    result.set("history", history);
    return result;
}

JsonValue ApiHandler::splitReport(const JsonValue& body)
{
    int groupId = (int)body.getNumberOr("groupId", -1);

    JsonValue result = JsonValue::makeObject();
    setMoney(result, "totalSpent", splitAnalytics.totalSpent(groupId));
    setMoney(result, "paidByYou", splitAnalytics.totalPaidBy(1, groupId));
    setMoney(result, "yourShare", splitAnalytics.totalShareOf(1, groupId));
    result.set("byCategory", totalsToJson(splitAnalytics.shareByCategory(1, groupId), "category"));
    return result;
}

JsonValue ApiHandler::activityList(const JsonValue& body)
{
    int limit = (int)body.getNumberOr("limit", 50);
    vector<Activity> recent = activity.getRecent(limit);

    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < (int)recent.size(); i++)
    {
        JsonValue item = JsonValue::makeObject();
        item.setNumber("id", recent[i].id);
        item.setString("date", recent[i].date.toString());
        item.setString("text", recent[i].text);
        list.add(item);
    }
    return list;
}

}
