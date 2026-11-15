#pragma once

#include <string>
#include <vector>

#include "ActivityLog.hpp"
#include "AnalyticsService.hpp"
#include "CalendarService.hpp"
#include "DebtCalculator.hpp"
#include "ExpenseManager.hpp"
#include "ExpenseScheduler.hpp"
#include "Group.hpp"
#include "HttpServer.hpp"
#include "Json.hpp"
#include "People.hpp"
#include "SearchEngine.hpp"
#include "SplitAnalytics.hpp"
#include "SplitManager.hpp"
#include "UndoRedoManager.hpp"

namespace expense_tracker
{

class ApiHandler : public RequestHandler
{
public:
    ApiHandler(const std::string& webFolder, const std::string& dataFolder);

    HttpResponse handleRequest(const HttpRequest& request);

private:
    HttpResponse handleApi(const HttpRequest& request);
    HttpResponse serveFile(const std::string& path);
    JsonValue runAction(const std::string& action, const JsonValue& body);

    JsonValue listExpenses(const JsonValue& body);
    JsonValue addExpense(const JsonValue& body);
    JsonValue updateExpense(const JsonValue& body);
    JsonValue deleteExpense(const JsonValue& body);
    JsonValue getExpense(const JsonValue& body);
    JsonValue searchExpenses(const JsonValue& body);
    JsonValue filterExpenses(const JsonValue& body);
    JsonValue expenseStats();
    JsonValue undoAction();
    JsonValue redoAction();
    JsonValue undoStatus();
    JsonValue listCategories();
    JsonValue addCategory(const JsonValue& body);

    JsonValue analyticsTotal(const JsonValue& body);
    JsonValue analyticsCategories(const JsonValue& body);
    JsonValue analyticsModes(const JsonValue& body);
    JsonValue analyticsSummary(const JsonValue& body);
    JsonValue analyticsTop(const JsonValue& body);
    JsonValue analyticsCompare(const JsonValue& body);
    JsonValue analyticsBudget(const JsonValue& body);
    JsonValue analyticsReport(const JsonValue& body);
    JsonValue analyticsPrefix(const JsonValue& body);
    JsonValue calendarDay(const JsonValue& body);
    JsonValue calendarMonth(const JsonValue& body);
    JsonValue calendarRange(const JsonValue& body);
    JsonValue addRecurring(const JsonValue& body);
    JsonValue listRecurring();
    JsonValue nextRecurring();
    JsonValue dueRecurring(const JsonValue& body);
    JsonValue decideRecurring(const JsonValue& body);

    JsonValue listPeople();
    JsonValue addPerson(const JsonValue& body);
    JsonValue renamePerson(const JsonValue& body);
    JsonValue removePerson(const JsonValue& body);
    JsonValue listGroups();
    JsonValue createGroup(const JsonValue& body);
    JsonValue addGroupMember(const JsonValue& body);
    JsonValue removeGroupMember(const JsonValue& body);
    JsonValue deleteGroup(const JsonValue& body);
    JsonValue getGroup(const JsonValue& body);
    JsonValue addShared(const JsonValue& body);
    JsonValue updateShared(const JsonValue& body);
    JsonValue deleteShared(const JsonValue& body);
    JsonValue restoreShared(const JsonValue& body);
    JsonValue listShared(const JsonValue& body);
    JsonValue addSettlement(const JsonValue& body);
    JsonValue deleteSettlement(const JsonValue& body);
    JsonValue balancesOverview();
    JsonValue balancesPerson(const JsonValue& body);
    JsonValue splitReport(const JsonValue& body);
    JsonValue activityList(const JsonValue& body);

    void saveToFiles();
    void loadFromFiles();

    std::string groupName(int groupId) const;
    long long netOf(int groupId, int personId) const;

    ExpenseManager personal;
    UndoRedoManager undoRedo;
    AnalyticsService analytics;
    SearchEngine search;
    CalendarService calendar;
    ExpenseScheduler scheduler;
    std::vector<Occurrence> pendingDue;
    PeopleManager people;
    GroupManager groups;
    SplitManager split;
    DebtCalculator debts;
    SplitAnalytics splitAnalytics;
    ActivityLog activity;
    std::string webFolder;
    std::string dataFolder;
    bool changed;
};

}
