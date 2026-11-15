@echo off
g++ -std=c++17 -Wall -Wextra -Iinclude ^
    src\analytics\AnalyticsService.cpp ^
    src\analytics\Budget.cpp ^
    src\analytics\SpendingReport.cpp ^
    src\api\ApiAnalytics.cpp ^
    src\api\ApiHandler.cpp ^
    src\api\ApiJson.cpp ^
    src\api\ApiPersonal.cpp ^
    src\api\ApiSplit.cpp ^
    src\api\ApiStorage.cpp ^
    src\calendar\CalendarService.cpp ^
    src\calendar\ExpenseScheduler.cpp ^
    src\calendar\RecurringExpense.cpp ^
    src\common\Date.cpp ^
    src\common\Money.cpp ^
    src\common\StringHelper.cpp ^
    src\expense\Category.cpp ^
    src\expense\Expense.cpp ^
    src\expense\ExpenseManager.cpp ^
    src\search\FilterCriteria.cpp ^
    src\search\SearchEngine.cpp ^
    src\search\UndoRedoManager.cpp ^
    src\server\HttpServer.cpp ^
    src\server\Json.cpp ^
    src\split\ActivityLog.cpp ^
    src\split\DebtCalculator.cpp ^
    src\split\Group.cpp ^
    src\split\People.cpp ^
    src\split\SharedExpense.cpp ^
    src\split\SplitAnalytics.cpp ^
    src\split\SplitManager.cpp ^
    src\main.cpp ^
    -o pbl_project.exe -lws2_32
if errorlevel 1 (echo Build failed) else (echo Done. Run pbl_project.exe from this folder.)
