#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ApiHandler.hpp"
#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

static int toInt(const string& text)
{
    if (!isOnlyDigits(text) || text.size() > 9)
    {
        throw ExpenseError("Wrong number in the saved data: " + text);
    }
    return stoi(text);
}

static long long toLongLong(const string& text)
{
    if (!isOnlyDigits(text) || text.size() > 15)
    {
        throw ExpenseError("Wrong number in the saved data: " + text);
    }
    return stoll(text);
}

// reads all lines of a txt file (an empty list if the file does not exist yet)
static vector<string> readLines(const string& path)
{
    vector<string> lines;
    ifstream file(path.c_str());
    if (!file.is_open())
    {
        return lines;
    }

    string line;
    while (getline(file, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        if (!line.empty())
        {
            lines.push_back(line);
        }
    }
    return lines;
}

static vector<string> readFields(const string& line, int expectedCount, const string& fileName)
{
    vector<string> parts = splitText(line, '|');
    if ((int)parts.size() != expectedCount)
    {
        throw ExpenseError("Wrong line in " + fileName);
    }
    return parts;
}

static void checkFileOpen(const ofstream& file, const string& path)
{
    if (!file.is_open())
    {
        throw ExpenseError("Cannot write the file " + path);
    }
}

// ---------------------------------------------------------------- saving

static void saveCategories(const string& folder, const CategoryList& categories)
{
    string filePath = folder + "/categories.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    for (int i = 0; i < categories.getCount(); i++)
    {
        file << encodeField(categories.getCategoryAt(i)) << "\n";
    }
}

static void saveExpenses(const string& folder, const ExpenseManager& personal)
{
    string filePath = folder + "/expenses.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    file << personal.getNextId() << "\n";
    for (int i = 0; i < personal.getCount(); i++)
    {
        const Expense& e = personal.getAllExpenses()[i];
        file << e.getId() << "|" << e.getAmount() << "|" << encodeField(e.getCategory()) << "|"
             << e.getDate().toString() << "|" << encodeField(e.getNote()) << "|" << encodeField(e.getPlace())
             << "|" << modeToString(e.getMode()) << "\n";
    }
}

static void saveRecurring(const string& folder, const ExpenseScheduler& scheduler, const vector<Occurrence>& pendingDue)
{
    string rulesPath = folder + "/recurring.txt";
    ofstream rules(rulesPath.c_str());
    checkFileOpen(rules, rulesPath);
    for (int i = 1; i <= scheduler.getRuleCount(); i++)
    {
        const RecurringExpense& rule = scheduler.getRule(i);
        ExpenseData data = rule.getData();
        string endText = "";
        if (rule.hasEndDate())
        {
            endText = rule.getEndDate().toString();
        }
        rules << data.amount << "|" << encodeField(data.category) << "|" << encodeField(data.note) << "|"
              << encodeField(data.place) << "|" << modeToString(data.mode) << "|"
              << frequencyToString(rule.getFrequency()) << "|" << rule.getStartDate().toString() << "|"
              << endText << "|" << scheduler.getTakenCount(i) << "\n";
    }

    string pendingPath = folder + "/pending.txt";
    ofstream pending(pendingPath.c_str());
    checkFileOpen(pending, pendingPath);
    for (int i = 0; i < (int)pendingDue.size(); i++)
    {
        pending << pendingDue[i].ruleId << "|" << pendingDue[i].index << "|" << pendingDue[i].due.toString() << "\n";
    }
}

static void savePeople(const string& folder, const PeopleManager& people)
{
    string filePath = folder + "/people.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    file << people.getNextId() << "\n";
    for (int i = 0; i < (int)people.getAll().size(); i++)
    {
        file << people.getAll()[i].id << "|" << encodeField(people.getAll()[i].name) << "\n";
    }
}

static void saveGroups(const string& folder, const GroupManager& groups)
{
    string filePath = folder + "/groups.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    file << groups.getNextId() << "\n";
    for (int i = 0; i < (int)groups.getAll().size(); i++)
    {
        const Group& group = groups.getAll()[i];
        file << group.id << "|" << encodeField(group.name) << "|";
        for (int j = 0; j < (int)group.members.size(); j++)
        {
            if (j > 0)
            {
                file << ",";
            }
            file << group.members[j];
        }
        file << "\n";
    }
}

static void saveShared(const string& folder, const SplitManager& split)
{
    string filePath = folder + "/shared.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    file << split.getNextExpenseId() << "\n";
    for (int i = 0; i < (int)split.getAllSharedExpenses().size(); i++)
    {
        const SharedExpense& expense = split.getAllSharedExpenses()[i];
        const SharedExpenseData& data = expense.getData();
        file << expense.getId() << "|" << data.groupId << "|" << encodeField(data.description) << "|"
             << encodeField(data.category) << "|" << data.amount << "|" << data.date.toString() << "|"
             << data.paidBy << "|" << splitTypeToString(data.splitType) << "|" << encodeField(data.note) << "|";
        if (expense.isDeleted())
        {
            file << "1|";
        }
        else
        {
            file << "0|";
        }
        for (int j = 0; j < (int)expense.getShares().size(); j++)
        {
            const SplitShare& share = expense.getShares()[j];
            if (j > 0)
            {
                file << ",";
            }
            file << share.personId << ":" << share.inputValue << ":" << share.owed;
        }
        file << "\n";
    }
}

static void saveSettlements(const string& folder, const SplitManager& split)
{
    string filePath = folder + "/settlements.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    file << split.getNextSettlementId() << "\n";
    for (int i = 0; i < (int)split.getAllSettlements().size(); i++)
    {
        const Settlement& settlement = split.getAllSettlements()[i];
        const SettlementData& data = settlement.getData();
        file << settlement.getId() << "|" << data.groupId << "|" << data.fromId << "|" << data.toId << "|"
             << data.amount << "|" << data.date.toString() << "|" << encodeField(data.note) << "|";
        if (settlement.isDeleted())
        {
            file << "1\n";
        }
        else
        {
            file << "0\n";
        }
    }
}

static void saveActivity(const string& folder, const ActivityLog& activity)
{
    string filePath = folder + "/activity.txt";
    ofstream file(filePath.c_str());
    checkFileOpen(file, filePath);
    file << activity.getNextId() << "\n";
    for (int i = 0; i < (int)activity.getAll().size(); i++)
    {
        file << activity.getAll()[i].id << "|" << activity.getAll()[i].date.toString() << "|"
             << encodeField(activity.getAll()[i].text) << "\n";
    }
}

void ApiHandler::saveToFiles()
{
    try
    {
        saveCategories(dataFolder, personal.getCategories());
        saveExpenses(dataFolder, personal);
        saveRecurring(dataFolder, scheduler, pendingDue);
        savePeople(dataFolder, people);
        saveGroups(dataFolder, groups);
        saveShared(dataFolder, split);
        saveSettlements(dataFolder, split);
        saveActivity(dataFolder, activity);
    }
    catch (const ExpenseError& error)
    {
        cerr << "Warning: " << error.what() << " (does the data folder exist?)" << endl;
    }
}

// ---------------------------------------------------------------- loading

static void loadCategories(const string& folder, CategoryList& categories)
{
    vector<string> lines = readLines(folder + "/categories.txt");
    for (int i = 0; i < (int)lines.size(); i++)
    {
        string name = decodeField(lines[i]);
        if (!categories.hasCategory(name))
        {
            categories.addCategory(name);
        }
    }
}

static void loadExpenses(const string& folder, ExpenseManager& personal)
{
    vector<string> lines = readLines(folder + "/expenses.txt");
    for (int i = 1; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 7, "expenses.txt");
        ExpenseData data;
        data.amount = toLongLong(parts[1]);
        data.category = decodeField(parts[2]);
        data.date = Date::parse(parts[3]);
        data.note = decodeField(parts[4]);
        data.place = decodeField(parts[5]);
        data.mode = stringToMode(parts[6]);
        personal.restoreExpense(Expense(toInt(parts[0]), data));
    }
    if (!lines.empty() && toInt(lines[0]) > personal.getNextId())
    {
        personal.setNextId(toInt(lines[0]));
    }
}

static void loadRecurring(const string& folder, ExpenseScheduler& scheduler, vector<Occurrence>& pendingDue)
{
    vector<string> lines = readLines(folder + "/recurring.txt");
    for (int i = 0; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 9, "recurring.txt");
        ExpenseData data;
        data.amount = toLongLong(parts[0]);
        data.category = decodeField(parts[1]);
        data.note = decodeField(parts[2]);
        data.place = decodeField(parts[3]);
        data.mode = stringToMode(parts[4]);
        Frequency frequency = DAILY;
        if (parts[5] == "weekly")
        {
            frequency = WEEKLY;
        }
        else if (parts[5] == "monthly")
        {
            frequency = MONTHLY;
        }
        Date startDate = Date::parse(parts[6]);
        int taken = toInt(parts[8]);

        if (parts[7].empty())
        {
            scheduler.addRuleWithProgress(RecurringExpense(data, frequency, startDate), taken);
        }
        else
        {
            scheduler.addRuleWithProgress(RecurringExpense(data, frequency, startDate, Date::parse(parts[7])), taken);
        }
    }

    vector<string> pendingLines = readLines(folder + "/pending.txt");
    for (int i = 0; i < (int)pendingLines.size(); i++)
    {
        vector<string> parts = readFields(pendingLines[i], 3, "pending.txt");
        Occurrence occurrence;
        occurrence.ruleId = toInt(parts[0]);
        occurrence.index = toInt(parts[1]);
        occurrence.due = Date::parse(parts[2]);
        pendingDue.push_back(occurrence);
    }
}

static void loadPeople(const string& folder, PeopleManager& people)
{
    vector<string> lines = readLines(folder + "/people.txt");
    if (lines.empty())
    {
        return;
    }
    people.clearAll();
    for (int i = 1; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 2, "people.txt");
        Person person;
        person.id = toInt(parts[0]);
        person.name = decodeField(parts[1]);
        people.loadPerson(person);
    }
    people.setNextId(toInt(lines[0]));
}

static void loadGroups(const string& folder, GroupManager& groups)
{
    vector<string> lines = readLines(folder + "/groups.txt");
    if (lines.empty())
    {
        return;
    }
    groups.clearAll();
    for (int i = 1; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 3, "groups.txt");
        Group group;
        group.id = toInt(parts[0]);
        group.name = decodeField(parts[1]);
        vector<string> members = splitText(parts[2], ',');
        for (int j = 0; j < (int)members.size(); j++)
        {
            group.members.push_back(toInt(members[j]));
        }
        groups.loadGroup(group);
    }
    groups.setNextId(toInt(lines[0]));
}

static void loadShared(const string& folder, SplitManager& split)
{
    vector<string> lines = readLines(folder + "/shared.txt");
    if (lines.empty())
    {
        return;
    }
    for (int i = 1; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 11, "shared.txt");
        SharedExpenseData data;
        data.groupId = toInt(parts[1]);
        data.description = decodeField(parts[2]);
        data.category = decodeField(parts[3]);
        data.amount = toLongLong(parts[4]);
        data.date = Date::parse(parts[5]);
        data.paidBy = toInt(parts[6]);
        data.splitType = stringToSplitType(parts[7]);
        data.note = decodeField(parts[8]);

        vector<SplitShare> shares;
        vector<string> shareTexts = splitText(parts[10], ',');
        for (int j = 0; j < (int)shareTexts.size(); j++)
        {
            vector<string> pieces = splitText(shareTexts[j], ':');
            if (pieces.size() != 3)
            {
                throw ExpenseError("Wrong line in shared.txt");
            }
            SplitShare share;
            share.personId = toInt(pieces[0]);
            share.inputValue = toLongLong(pieces[1]);
            share.owed = toLongLong(pieces[2]);
            shares.push_back(share);

            SplitInput input;
            input.personId = share.personId;
            input.value = share.inputValue;
            data.inputs.push_back(input);
        }

        SharedExpense expense(toInt(parts[0]), data, shares);
        expense.setDeleted(parts[9] == "1");
        split.loadSharedExpense(expense);
    }
    split.setNextExpenseId(toInt(lines[0]));
}

static void loadSettlements(const string& folder, SplitManager& split)
{
    vector<string> lines = readLines(folder + "/settlements.txt");
    if (lines.empty())
    {
        return;
    }
    for (int i = 1; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 8, "settlements.txt");
        SettlementData data;
        data.groupId = toInt(parts[1]);
        data.fromId = toInt(parts[2]);
        data.toId = toInt(parts[3]);
        data.amount = toLongLong(parts[4]);
        data.date = Date::parse(parts[5]);
        data.note = decodeField(parts[6]);

        Settlement settlement(toInt(parts[0]), data);
        settlement.setDeleted(parts[7] == "1");
        split.loadSettlement(settlement);
    }
    split.setNextSettlementId(toInt(lines[0]));
}

static void loadActivity(const string& folder, ActivityLog& activity)
{
    vector<string> lines = readLines(folder + "/activity.txt");
    if (lines.empty())
    {
        return;
    }
    for (int i = 1; i < (int)lines.size(); i++)
    {
        vector<string> parts = readFields(lines[i], 3, "activity.txt");
        Activity item;
        item.id = toInt(parts[0]);
        item.date = Date::parse(parts[1]);
        item.text = decodeField(parts[2]);
        activity.loadActivity(item);
    }
    activity.setNextId(toInt(lines[0]));
}

void ApiHandler::loadFromFiles()
{
    try
    {
        loadPeople(dataFolder, people);
        loadGroups(dataFolder, groups);
        loadCategories(dataFolder, personal.getCategories());
        loadExpenses(dataFolder, personal);
        loadRecurring(dataFolder, scheduler, pendingDue);
        loadShared(dataFolder, split);
        loadSettlements(dataFolder, split);
        loadActivity(dataFolder, activity);
        cout << "Saved data loaded from the " << dataFolder << " folder." << endl;
    }
    catch (const ExpenseError& error)
    {
        cerr << "Warning: could not read the saved data (" << error.what() << ")" << endl;
    }
}

}
