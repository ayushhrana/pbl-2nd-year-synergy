#pragma once

#include <stack>

#include "ExpenseManager.hpp"

namespace expense_tracker
{

class UndoRedoManager
{
public:
    UndoRedoManager(ExpenseManager& manager) : manager(manager)
    {
    }

    int addExpense(const ExpenseData& data);
    Expense deleteExpense(int id);
    void updateExpense(int id, const ExpenseData& data);

    void undo();
    void redo();

    bool canUndo() const
    {
        return !undoStack.empty();
    }

    bool canRedo() const
    {
        return !redoStack.empty();
    }

private:
    enum ActionType
    {
        ADDED,
        DELETED,
        UPDATED
    };

    struct Action
    {
        ActionType type;
        Expense before;
        Expense after;

        Action(ActionType t, const Expense& b, const Expense& a) : type(t), before(b), after(a)
        {
        }
    };

    void saveAction(const Action& action);
    void applyAction(const Action& action, bool forward);

    ExpenseManager& manager;
    std::stack<Action> undoStack;
    std::stack<Action> redoStack;
};

}
