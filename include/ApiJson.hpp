#pragma once

#include <string>
#include <vector>

#include "DebtCalculator.hpp"
#include "Expense.hpp"
#include "ExpenseScheduler.hpp"
#include "Json.hpp"
#include "SharedExpense.hpp"
#include "Settlement.hpp"

namespace expense_tracker
{

Frequency readFrequency(const std::string& text);
void setMoney(JsonValue& object, const std::string& key, long long paise);
Date readDate(const JsonValue& body, const std::string& key);
ExpenseData readExpenseData(const JsonValue& body, bool needDate);
SharedExpenseData readSharedData(const JsonValue& body);
SettlementData readSettlementData(const JsonValue& body);

JsonValue expenseToJson(const Expense& expense);
JsonValue expensesToJson(const std::vector<Expense>& expenses);
JsonValue personToJson(const Person& person);
JsonValue groupToJson(const Group& group, const PeopleManager& people);
JsonValue sharedExpenseToJson(const SharedExpense& expense, const PeopleManager& people, const GroupManager& groups);
JsonValue settlementToJson(const Settlement& settlement, const PeopleManager& people);
JsonValue debtToJson(const Debt& debt, const PeopleManager& people);
JsonValue recurringToJson(const RecurringExpense& rule, int ruleId, int takenCount);
JsonValue occurrenceToJson(const Occurrence& occurrence, const RecurringExpense& rule);
JsonValue totalsToJson(const std::map<std::string, long long>& totals, const std::string& keyName);

}
