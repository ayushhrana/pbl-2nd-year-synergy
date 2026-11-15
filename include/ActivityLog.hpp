#pragma once

#include <string>
#include <vector>

#include "Date.hpp"

namespace expense_tracker
{

struct Activity
{
    int id = 0;
    Date date;
    std::string text;
};

class ActivityLog
{
public:
    ActivityLog();

    void add(const std::string& text);
    std::vector<Activity> getRecent(int limit) const;
    void loadActivity(const Activity& activity);

    const std::vector<Activity>& getAll() const
    {
        return activities;
    }

    int getNextId() const
    {
        return nextId;
    }

    void setNextId(int value)
    {
        nextId = value;
    }

    void clearAll()
    {
        activities.clear();
    }

private:
    std::vector<Activity> activities;
    int nextId;
};

}
