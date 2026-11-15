#include "ApiHandler.hpp"

#include <fstream>
#include <sstream>

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

ApiHandler::ApiHandler(const string& webFolder, const string& dataFolder)
    : undoRedo(personal), analytics(personal), search(personal), calendar(personal),
      split(people, groups, personal.getCategories()), debts(split, groups), splitAnalytics(split)
{
    this->webFolder = webFolder;
    this->dataFolder = dataFolder;
    changed = false;
    loadFromFiles();
}

HttpResponse ApiHandler::handleRequest(const HttpRequest& request)
{
    if (request.path.compare(0, 5, "/api/") == 0)
    {
        return handleApi(request);
    }
    return serveFile(request.path);
}

HttpResponse ApiHandler::handleApi(const HttpRequest& request)
{
    HttpResponse response;
    response.contentType = "application/json";
    JsonValue answer = JsonValue::makeObject();

    try
    {
        string action = request.path.substr(5);
        string text = trimSpaces(request.body);
        if (text.empty())
        {
            text = "{}";
        }
        JsonValue body = JsonValue::parse(text);

        JsonValue data = runAction(action, body);
        answer.setBool("ok", true);
        answer.set("data", data);

        if (changed)
        {
            saveToFiles();
            changed = false;
        }
    }
    catch (const ExpenseError& error)
    {
        answer = JsonValue::makeObject();
        answer.setBool("ok", false);
        answer.setString("error", error.what());
    }
    catch (const exception& error)
    {
        answer = JsonValue::makeObject();
        answer.setBool("ok", false);
        answer.setString("error", string("Something went wrong: ") + error.what());
    }

    response.body = answer.toString();
    return response;
}

static string contentTypeFor(const string& path)
{
    if (path.size() >= 5 && path.compare(path.size() - 5, 5, ".html") == 0)
    {
        return "text/html; charset=utf-8";
    }
    if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".css") == 0)
    {
        return "text/css; charset=utf-8";
    }
    if (path.size() >= 3 && path.compare(path.size() - 3, 3, ".js") == 0)
    {
        return "application/javascript; charset=utf-8";
    }
    return "text/plain; charset=utf-8";
}

HttpResponse ApiHandler::serveFile(const string& path)
{
    HttpResponse response;

    string cleanPath = path;
    if (cleanPath == "/")
    {
        cleanPath = "/index.html";
    }
    if (cleanPath.find("..") != string::npos || cleanPath.find('\\') != string::npos)
    {
        response.status = 404;
        response.body = "Not found";
        return response;
    }

    ifstream file((webFolder + cleanPath).c_str(), ios::binary);
    if (!file.is_open())
    {
        response.status = 404;
        response.body = "Not found";
        return response;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    response.contentType = contentTypeFor(cleanPath);
    response.body = buffer.str();
    return response;
}

JsonValue ApiHandler::runAction(const string& action, const JsonValue& body)
{
    if (action == "expenses/list")
    {
        return listExpenses(body);
    }
    if (action == "expenses/add")
    {
        return addExpense(body);
    }
    if (action == "expenses/update")
    {
        return updateExpense(body);
    }
    if (action == "expenses/delete")
    {
        return deleteExpense(body);
    }
    if (action == "expenses/get")
    {
        return getExpense(body);
    }
    if (action == "expenses/search")
    {
        return searchExpenses(body);
    }
    if (action == "expenses/filter")
    {
        return filterExpenses(body);
    }
    if (action == "expenses/stats")
    {
        return expenseStats();
    }
    if (action == "undo")
    {
        return undoAction();
    }
    if (action == "redo")
    {
        return redoAction();
    }
    if (action == "undo/status")
    {
        return undoStatus();
    }
    if (action == "categories/list")
    {
        return listCategories();
    }
    if (action == "categories/add")
    {
        return addCategory(body);
    }

    if (action == "analytics/total")
    {
        return analyticsTotal(body);
    }
    if (action == "analytics/categories")
    {
        return analyticsCategories(body);
    }
    if (action == "analytics/modes")
    {
        return analyticsModes(body);
    }
    if (action == "analytics/summary")
    {
        return analyticsSummary(body);
    }
    if (action == "analytics/top")
    {
        return analyticsTop(body);
    }
    if (action == "analytics/compare")
    {
        return analyticsCompare(body);
    }
    if (action == "analytics/budget")
    {
        return analyticsBudget(body);
    }
    if (action == "analytics/report")
    {
        return analyticsReport(body);
    }
    if (action == "analytics/prefix")
    {
        return analyticsPrefix(body);
    }
    if (action == "calendar/day")
    {
        return calendarDay(body);
    }
    if (action == "calendar/month")
    {
        return calendarMonth(body);
    }
    if (action == "calendar/range")
    {
        return calendarRange(body);
    }
    if (action == "recurring/add")
    {
        return addRecurring(body);
    }
    if (action == "recurring/list")
    {
        return listRecurring();
    }
    if (action == "recurring/next")
    {
        return nextRecurring();
    }
    if (action == "recurring/due")
    {
        return dueRecurring(body);
    }
    if (action == "recurring/decide")
    {
        return decideRecurring(body);
    }

    if (action == "people/list")
    {
        return listPeople();
    }
    if (action == "people/add")
    {
        return addPerson(body);
    }
    if (action == "people/rename")
    {
        return renamePerson(body);
    }
    if (action == "people/remove")
    {
        return removePerson(body);
    }
    if (action == "groups/list")
    {
        return listGroups();
    }
    if (action == "groups/create")
    {
        return createGroup(body);
    }
    if (action == "groups/addMember")
    {
        return addGroupMember(body);
    }
    if (action == "groups/removeMember")
    {
        return removeGroupMember(body);
    }
    if (action == "groups/delete")
    {
        return deleteGroup(body);
    }
    if (action == "groups/get")
    {
        return getGroup(body);
    }
    if (action == "shared/add")
    {
        return addShared(body);
    }
    if (action == "shared/update")
    {
        return updateShared(body);
    }
    if (action == "shared/delete")
    {
        return deleteShared(body);
    }
    if (action == "shared/restore")
    {
        return restoreShared(body);
    }
    if (action == "shared/list")
    {
        return listShared(body);
    }
    if (action == "settle/add")
    {
        return addSettlement(body);
    }
    if (action == "settle/delete")
    {
        return deleteSettlement(body);
    }
    if (action == "balances/overview")
    {
        return balancesOverview();
    }
    if (action == "balances/person")
    {
        return balancesPerson(body);
    }
    if (action == "split/report")
    {
        return splitReport(body);
    }
    if (action == "activity")
    {
        return activityList(body);
    }

    if (action == "today")
    {
        JsonValue result = JsonValue::makeObject();
        result.setString("today", Date::today().toString());
        return result;
    }

    throw ExpenseError("Unknown action: " + action);
}

}
