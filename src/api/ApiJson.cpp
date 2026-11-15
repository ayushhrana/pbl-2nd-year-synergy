#include "ApiJson.hpp"

#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

Frequency readFrequency(const string& text)
{
    string clean = toLowerCase(trimSpaces(text));
    if (clean == "daily")
    {
        return DAILY;
    }
    if (clean == "weekly")
    {
        return WEEKLY;
    }
    if (clean == "monthly")
    {
        return MONTHLY;
    }
    throw ExpenseError("Frequency must be daily, weekly or monthly");
}

void setMoney(JsonValue& object, const string& key, long long paise)
{
    object.setNumber(key, paise);
    object.setString(key + "Text", paiseToRupees(paise));
}

Date readDate(const JsonValue& body, const string& key)
{
    return Date::parse(body.getString(key));
}

static string readText(const JsonValue& body, const string& key)
{
    const JsonValue& value = body.get(key);
    if (value.getType() == JSON_NUMBER)
    {
        return to_string(value.asNumber());
    }
    return value.asString();
}

ExpenseData readExpenseData(const JsonValue& body, bool needDate)
{
    ExpenseData data;
    data.amount = rupeesToPaise(readText(body, "amount"));
    data.category = body.getString("category");
    if (needDate)
    {
        data.date = readDate(body, "date");
    }
    data.note = body.getStringOr("note", "");
    data.place = body.getStringOr("place", "");
    data.mode = stringToMode(body.getStringOr("mode", "cash"));
    return data;
}

SharedExpenseData readSharedData(const JsonValue& body)
{
    SharedExpenseData data;
    data.groupId = (int)body.getNumberOr("groupId", 0);
    data.description = body.getString("description");
    data.category = body.getString("category");
    data.amount = rupeesToPaise(readText(body, "amount"));
    data.date = readDate(body, "date");
    data.paidBy = (int)body.getNumber("paidBy");
    data.splitType = stringToSplitType(body.getStringOr("splitType", "equal"));
    data.note = body.getStringOr("note", "");

    const JsonValue& inputs = body.get("inputs");
    for (int i = 0; i < inputs.size(); i++)
    {
        SplitInput input;
        input.personId = (int)inputs.at(i).getNumber("personId");

        if (data.splitType == SPLIT_EXACT || data.splitType == SPLIT_PERCENT || data.splitType == SPLIT_SHARES)
        {
            if (trimSpaces(readText(inputs.at(i), "value")).empty())
            {
                throw ExpenseError("Enter a value for every selected person");
            }
        }

        if (data.splitType == SPLIT_EXACT || data.splitType == SPLIT_PERCENT)
        {
            input.value = parseHundredths(trimSpaces(readText(inputs.at(i), "value")));
        }
        else if (data.splitType == SPLIT_SHARES)
        {
            string text = trimSpaces(readText(inputs.at(i), "value"));
            if (!isOnlyDigits(text) || text.size() > 6)
            {
                throw ExpenseError("Shares must be whole numbers");
            }
            input.value = stoll(text);
        }
        data.inputs.push_back(input);
    }
    return data;
}

SettlementData readSettlementData(const JsonValue& body)
{
    SettlementData data;
    data.groupId = (int)body.getNumberOr("groupId", 0);
    data.fromId = (int)body.getNumber("fromId");
    data.toId = (int)body.getNumber("toId");
    data.amount = rupeesToPaise(readText(body, "amount"));
    data.date = readDate(body, "date");
    data.note = body.getStringOr("note", "");
    return data;
}

JsonValue expenseToJson(const Expense& expense)
{
    JsonValue json = JsonValue::makeObject();
    json.setNumber("id", expense.getId());
    setMoney(json, "amount", expense.getAmount());
    json.setString("category", expense.getCategory());
    json.setString("date", expense.getDate().toString());
    json.setString("note", expense.getNote());
    json.setString("place", expense.getPlace());
    json.setString("mode", modeToString(expense.getMode()));
    return json;
}

JsonValue expensesToJson(const vector<Expense>& expenses)
{
    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < (int)expenses.size(); i++)
    {
        list.add(expenseToJson(expenses[i]));
    }
    return list;
}

JsonValue personToJson(const Person& person)
{
    JsonValue json = JsonValue::makeObject();
    json.setNumber("id", person.id);
    json.setString("name", person.name);
    return json;
}

JsonValue groupToJson(const Group& group, const PeopleManager& people)
{
    JsonValue json = JsonValue::makeObject();
    json.setNumber("id", group.id);
    json.setString("name", group.name);

    JsonValue members = JsonValue::makeArray();
    for (int i = 0; i < (int)group.members.size(); i++)
    {
        JsonValue member = JsonValue::makeObject();
        member.setNumber("id", group.members[i]);
        member.setString("name", people.getName(group.members[i]));
        members.add(member);
    }
    json.set("members", members);
    return json;
}

JsonValue sharedExpenseToJson(const SharedExpense& expense, const PeopleManager& people, const GroupManager& groups)
{
    const SharedExpenseData& data = expense.getData();

    JsonValue json = JsonValue::makeObject();
    json.setNumber("id", expense.getId());
    json.setNumber("groupId", data.groupId);
    if (data.groupId > 0 && groups.findGroup(data.groupId) != nullptr)
    {
        json.setString("groupName", groups.findGroup(data.groupId)->name);
    }
    else
    {
        json.setString("groupName", "No group");
    }
    json.setString("description", data.description);
    json.setString("category", data.category);
    setMoney(json, "amount", data.amount);
    json.setString("date", data.date.toString());
    json.setNumber("paidBy", data.paidBy);
    json.setString("paidByName", people.getName(data.paidBy));
    json.setString("splitType", splitTypeToString(data.splitType));
    json.setString("note", data.note);
    json.setBool("deleted", expense.isDeleted());

    JsonValue shares = JsonValue::makeArray();
    for (int i = 0; i < (int)expense.getShares().size(); i++)
    {
        const SplitShare& share = expense.getShares()[i];
        JsonValue item = JsonValue::makeObject();
        item.setNumber("personId", share.personId);
        item.setString("name", people.getName(share.personId));
        item.setNumber("inputValue", share.inputValue);
        setMoney(item, "owed", share.owed);
        shares.add(item);
    }
    json.set("shares", shares);
    return json;
}

JsonValue settlementToJson(const Settlement& settlement, const PeopleManager& people)
{
    const SettlementData& data = settlement.getData();

    JsonValue json = JsonValue::makeObject();
    json.setNumber("id", settlement.getId());
    json.setNumber("groupId", data.groupId);
    json.setNumber("fromId", data.fromId);
    json.setString("fromName", people.getName(data.fromId));
    json.setNumber("toId", data.toId);
    json.setString("toName", people.getName(data.toId));
    setMoney(json, "amount", data.amount);
    json.setString("date", data.date.toString());
    json.setString("note", data.note);
    json.setBool("deleted", settlement.isDeleted());
    return json;
}

JsonValue debtToJson(const Debt& debt, const PeopleManager& people)
{
    JsonValue json = JsonValue::makeObject();
    json.setNumber("fromId", debt.fromId);
    json.setString("fromName", people.getName(debt.fromId));
    json.setNumber("toId", debt.toId);
    json.setString("toName", people.getName(debt.toId));
    setMoney(json, "amount", debt.amount);
    return json;
}

JsonValue recurringToJson(const RecurringExpense& rule, int ruleId, int takenCount)
{
    ExpenseData data = rule.getData();

    JsonValue json = JsonValue::makeObject();
    json.setNumber("id", ruleId);
    setMoney(json, "amount", data.amount);
    json.setString("category", data.category);
    json.setString("note", data.note);
    json.setString("place", data.place);
    json.setString("mode", modeToString(data.mode));
    json.setString("frequency", frequencyToString(rule.getFrequency()));
    json.setString("start", rule.getStartDate().toString());
    json.setBool("hasEnd", rule.hasEndDate());
    if (rule.hasEndDate())
    {
        json.setString("end", rule.getEndDate().toString());
    }
    else
    {
        json.setString("end", "");
    }
    json.setNumber("taken", takenCount);
    return json;
}

JsonValue occurrenceToJson(const Occurrence& occurrence, const RecurringExpense& rule)
{
    ExpenseData data = rule.getData();

    JsonValue json = JsonValue::makeObject();
    json.setNumber("ruleId", occurrence.ruleId);
    json.setNumber("index", occurrence.index);
    json.setString("due", occurrence.due.toString());
    setMoney(json, "amount", data.amount);
    json.setString("category", data.category);
    json.setString("note", data.note);
    json.setString("place", data.place);
    json.setString("mode", modeToString(data.mode));
    return json;
}

JsonValue totalsToJson(const map<string, long long>& totals, const string& keyName)
{
    JsonValue list = JsonValue::makeArray();
    map<string, long long>::const_iterator it;
    for (it = totals.begin(); it != totals.end(); ++it)
    {
        JsonValue item = JsonValue::makeObject();
        item.setString(keyName, it->first);
        setMoney(item, "total", it->second);
        list.add(item);
    }
    return list;
}

}
