#include "ApiHandler.hpp"

#include "ApiJson.hpp"
#include "Errors.hpp"
#include "Money.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

JsonValue ApiHandler::analyticsTotal(const JsonValue& body)
{
    JsonValue result = JsonValue::makeObject();
    setMoney(result, "total", analytics.totalBetween(readDate(body, "from"), readDate(body, "to")));
    return result;
}

JsonValue ApiHandler::analyticsCategories(const JsonValue& body)
{
    return totalsToJson(analytics.totalByCategory(readDate(body, "from"), readDate(body, "to")), "category");
}

JsonValue ApiHandler::analyticsModes(const JsonValue& body)
{
    map<PaymentMode, long long> totals = analytics.totalByMode(readDate(body, "from"), readDate(body, "to"));

    JsonValue list = JsonValue::makeArray();
    map<PaymentMode, long long>::iterator it;
    for (it = totals.begin(); it != totals.end(); ++it)
    {
        JsonValue item = JsonValue::makeObject();
        item.setString("mode", modeToString(it->first));
        setMoney(item, "total", it->second);
        list.add(item);
    }
    return list;
}

JsonValue ApiHandler::analyticsSummary(const JsonValue& body)
{
    Date fromDate = readDate(body, "from");
    Date toDate = readDate(body, "to");
    string period = body.getStringOr("period", "monthly");

    map<string, long long> totals;
    if (period == "daily")
    {
        totals = analytics.dailySummary(fromDate, toDate);
    }
    else if (period == "weekly")
    {
        totals = analytics.weeklySummary(fromDate, toDate);
    }
    else
    {
        totals = analytics.monthlySummary(fromDate, toDate);
    }
    return totalsToJson(totals, "period");
}

JsonValue ApiHandler::analyticsTop(const JsonValue& body)
{
    vector<CategoryTotal> top = analytics.topCategories(readDate(body, "from"), readDate(body, "to"),
                                                        (int)body.getNumberOr("n", 3));
    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < (int)top.size(); i++)
    {
        JsonValue item = JsonValue::makeObject();
        item.setString("category", top[i].category);
        setMoney(item, "total", top[i].total);
        list.add(item);
    }
    return list;
}

JsonValue ApiHandler::analyticsCompare(const JsonValue& body)
{
    PeriodComparison result = analytics.comparePeriods(readDate(body, "aFrom"), readDate(body, "aTo"),
                                                       readDate(body, "bFrom"), readDate(body, "bTo"));
    JsonValue json = JsonValue::makeObject();
    setMoney(json, "first", result.firstTotal);
    setMoney(json, "second", result.secondTotal);
    setMoney(json, "difference", result.difference);
    json.setString("trend", result.trend);
    json.setBool("hasPercent", result.hasPercent);
    json.setNumber("percent", result.percentChange);
    return json;
}

JsonValue ApiHandler::analyticsBudget(const JsonValue& body)
{
    long long limit = rupeesToPaise(body.getString("limit"));
    Budget budget(limit, body.getStringOr("category", ""));
    BudgetStatus status = analytics.budgetStatus(budget, readDate(body, "from"), readDate(body, "to"));

    JsonValue json = JsonValue::makeObject();
    setMoney(json, "limit", status.limit);
    setMoney(json, "spent", status.spent);
    setMoney(json, "remaining", status.remaining);
    json.setBool("exceeded", status.exceeded);
    return json;
}

JsonValue ApiHandler::analyticsReport(const JsonValue& body)
{
    SpendingReport report = analytics.makeReport(readDate(body, "from"), readDate(body, "to"));

    JsonValue json = JsonValue::makeObject();
    json.setString("from", report.getFromDate().toString());
    json.setString("to", report.getToDate().toString());
    json.setNumber("count", report.getExpenseCount());
    json.setNumber("days", report.getNumberOfDays());
    setMoney(json, "total", report.getTotalSpent());
    setMoney(json, "averagePerDay", report.getAveragePerDay());
    json.setString("topCategory", report.getTopCategory());
    json.set("categories", totalsToJson(report.getCategoryTotals(), "category"));

    JsonValue modes = JsonValue::makeArray();
    map<PaymentMode, long long>::const_iterator it;
    for (it = report.getModeTotals().begin(); it != report.getModeTotals().end(); ++it)
    {
        JsonValue item = JsonValue::makeObject();
        item.setString("mode", modeToString(it->first));
        setMoney(item, "total", it->second);
        modes.add(item);
    }
    json.set("modes", modes);
    return json;
}

JsonValue ApiHandler::analyticsPrefix(const JsonValue& body)
{
    Date fromDate = readDate(body, "from");
    Date toDate = readDate(body, "to");
    Date queryFrom = readDate(body, "queryFrom");
    Date queryTo = readDate(body, "queryTo");

    vector<long long> prefix = analytics.makeDailyPrefixSums(fromDate, toDate);
    int startDay = queryFrom.getDayNumber() - fromDate.getDayNumber();
    int endDay = queryTo.getDayNumber() - fromDate.getDayNumber();

    JsonValue json = JsonValue::makeObject();
    setMoney(json, "total", analytics.totalFromPrefix(prefix, startDay, endDay));
    json.setNumber("days", (int)prefix.size() - 1);
    return json;
}

JsonValue ApiHandler::calendarDay(const JsonValue& body)
{
    return expensesToJson(calendar.getExpensesOnDay(readDate(body, "date")));
}

JsonValue ApiHandler::calendarMonth(const JsonValue& body)
{
    return expensesToJson(calendar.getExpensesInMonth((int)body.getNumber("year"), (int)body.getNumber("month")));
}

JsonValue ApiHandler::calendarRange(const JsonValue& body)
{
    return expensesToJson(calendar.getExpensesInRange(readDate(body, "from"), readDate(body, "to")));
}

JsonValue ApiHandler::addRecurring(const JsonValue& body)
{
    ExpenseData data = readExpenseData(body, false);
    data.category = personal.getCategories().findCategory(data.category);
    Frequency frequency = readFrequency(body.getString("frequency"));
    Date startDate = readDate(body, "start");

    int ruleId = 0;
    string endText = trimSpaces(body.getStringOr("end", ""));
    if (endText.empty())
    {
        ruleId = scheduler.addRule(RecurringExpense(data, frequency, startDate));
    }
    else
    {
        ruleId = scheduler.addRule(RecurringExpense(data, frequency, startDate, Date::parse(endText)));
    }

    changed = true;
    activity.add("You set up a " + frequencyToString(frequency) + " recurring expense (" + data.note + ", " + paiseToRupees(data.amount) + ")");

    JsonValue result = JsonValue::makeObject();
    result.setNumber("id", ruleId);
    return result;
}

JsonValue ApiHandler::listRecurring()
{
    JsonValue list = JsonValue::makeArray();
    for (int i = 1; i <= scheduler.getRuleCount(); i++)
    {
        list.add(recurringToJson(scheduler.getRule(i), i, scheduler.getTakenCount(i)));
    }
    return list;
}

JsonValue ApiHandler::nextRecurring()
{
    JsonValue result = JsonValue::makeObject();
    Occurrence next;
    if (scheduler.getNextOccurrence(next))
    {
        result.setBool("has", true);
        result.set("occurrence", occurrenceToJson(next, scheduler.getRule(next.ruleId)));
    }
    else
    {
        result.setBool("has", false);
    }
    return result;
}

JsonValue ApiHandler::dueRecurring(const JsonValue& body)
{
    Date today = Date::today();
    if (body.has("today") && !trimSpaces(body.getString("today")).empty())
    {
        today = readDate(body, "today");
    }

    vector<Occurrence> newlyDue = scheduler.takeDueOccurrences(today);
    for (int i = 0; i < (int)newlyDue.size(); i++)
    {
        pendingDue.push_back(newlyDue[i]);
    }
    if (!newlyDue.empty())
    {
        changed = true;
    }

    JsonValue list = JsonValue::makeArray();
    for (int i = 0; i < (int)pendingDue.size(); i++)
    {
        list.add(occurrenceToJson(pendingDue[i], scheduler.getRule(pendingDue[i].ruleId)));
    }
    return list;
}

JsonValue ApiHandler::decideRecurring(const JsonValue& body)
{
    int ruleId = (int)body.getNumber("ruleId");
    int index = (int)body.getNumber("index");
    bool accept = body.getBoolOr("accept", false);

    for (int i = 0; i < (int)pendingDue.size(); i++)
    {
        if (pendingDue[i].ruleId == ruleId && pendingDue[i].index == index)
        {
            Occurrence occurrence = pendingDue[i];
            if (accept)
            {
                ExpenseData data = scheduler.getRule(ruleId).makeExpenseData(occurrence.due);
                int id = undoRedo.addExpense(data);
                activity.add("You recorded recurring expense #" + to_string(id) + " for " + occurrence.due.toString());
            }
            pendingDue.erase(pendingDue.begin() + i);
            changed = true;
            return JsonValue();
        }
    }
    throw ExpenseError("This recurring expense is not waiting for confirmation");
}

}
