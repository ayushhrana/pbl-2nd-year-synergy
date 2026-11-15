#include "UndoRedoManager.hpp"

#include "Errors.hpp"

using namespace std;

namespace expense_tracker
{

// every new action clears the redo stack
void UndoRedoManager::saveAction(const Action& action)
{
    undoStack.push(action);
    while (!redoStack.empty())
    {
        redoStack.pop();
    }
}

int UndoRedoManager::addExpense(const ExpenseData& data)
{
    int id = manager.addExpense(data);
    Expense added = *manager.findExpenseById(id);
    saveAction(Action(ADDED, added, added));
    return id;
}

Expense UndoRedoManager::deleteExpense(int id)
{
    Expense deleted = manager.deleteExpense(id);
    saveAction(Action(DELETED, deleted, deleted));
    return deleted;
}

void UndoRedoManager::updateExpense(int id, const ExpenseData& data)
{
    const Expense* oldExpense = manager.findExpenseById(id);
    if (oldExpense == nullptr)
    {
        throw ExpenseError("No expense found with ID " + to_string(id));
    }

    Expense before = *oldExpense;
    manager.updateExpense(id, data);
    Expense after = *manager.findExpenseById(id);
    saveAction(Action(UPDATED, before, after));
}

// forward = true repeats the action (redo), forward = false reverses it (undo)
// undo never makes a new ID, it puts back the same expense with the same ID
void UndoRedoManager::applyAction(const Action& action, bool forward)
{
    if (action.type == ADDED)
    {
        if (forward)
        {
            manager.restoreExpense(action.after);
        }
        else
        {
            manager.deleteExpense(action.after.getId());
        }
    }
    else if (action.type == DELETED)
    {
        if (forward)
        {
            manager.deleteExpense(action.before.getId());
        }
        else
        {
            manager.restoreExpense(action.before);
        }
    }
    else
    {
        if (forward)
        {
            manager.updateExpense(action.after.getId(), action.after.getData());
        }
        else
        {
            manager.updateExpense(action.before.getId(), action.before.getData());
        }
    }
}

void UndoRedoManager::undo()
{
    if (undoStack.empty())
    {
        throw ExpenseError("Nothing to undo");
    }

    Action action = undoStack.top();
    applyAction(action, false);
    undoStack.pop();
    redoStack.push(action);
}

void UndoRedoManager::redo()
{
    if (redoStack.empty())
    {
        throw ExpenseError("Nothing to redo");
    }

    Action action = redoStack.top();
    applyAction(action, true);
    redoStack.pop();
    undoStack.push(action);
}

}
