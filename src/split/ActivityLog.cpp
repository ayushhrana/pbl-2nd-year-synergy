#include "ActivityLog.hpp"

using namespace std;

namespace expense_tracker
{

ActivityLog::ActivityLog()
{
    nextId = 1;
}

void ActivityLog::add(const string& text)
{
    Activity activity;
    activity.id = nextId;
    activity.date = Date::today();
    activity.text = text;
    activities.push_back(activity);
    nextId = nextId + 1;

    if (activities.size() > 500)
    {
        activities.erase(activities.begin());
    }
}

vector<Activity> ActivityLog::getRecent(int limit) const
{
    vector<Activity> result;
    for (int i = (int)activities.size() - 1; i >= 0 && (int)result.size() < limit; i--)
    {
        result.push_back(activities[i]);
    }
    return result;
}

void ActivityLog::loadActivity(const Activity& activity)
{
    activities.push_back(activity);
}

}
