#include "Group.hpp"

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

GroupManager::GroupManager()
{
    nextId = 1;
}

int GroupManager::createGroup(const string& name, const vector<int>& members, const PeopleManager& people)
{
    string cleanName = trimSpaces(name);
    if (cleanName.empty())
    {
        throw ExpenseError("Group name cannot be empty");
    }
    if (cleanName.size() > 40)
    {
        throw ExpenseError("Group name is too long (maximum 40 letters)");
    }
    for (int i = 0; i < (int)groups.size(); i++)
    {
        if (isSameText(groups[i].name, cleanName))
        {
            throw ExpenseError("A group named " + cleanName + " already exists");
        }
    }

    Group group;
    group.id = nextId;
    group.name = cleanName;
    group.members.push_back(1);

    for (int i = 0; i < (int)members.size(); i++)
    {
        if (people.findPerson(members[i]) == nullptr)
        {
            throw ExpenseError("No person found with ID " + to_string(members[i]));
        }
        bool alreadyAdded = false;
        for (int j = 0; j < (int)group.members.size(); j++)
        {
            if (group.members[j] == members[i])
            {
                alreadyAdded = true;
            }
        }
        if (!alreadyAdded)
        {
            group.members.push_back(members[i]);
        }
    }

    groups.push_back(group);
    nextId = nextId + 1;
    return group.id;
}

void GroupManager::addMember(int groupId, int personId, const PeopleManager& people)
{
    if (people.findPerson(personId) == nullptr)
    {
        throw ExpenseError("No person found with ID " + to_string(personId));
    }
    for (int i = 0; i < (int)groups.size(); i++)
    {
        if (groups[i].id == groupId)
        {
            if (isMember(groupId, personId))
            {
                throw ExpenseError("This person is already in the group");
            }
            groups[i].members.push_back(personId);
            return;
        }
    }
    throw ExpenseError("No group found with ID " + to_string(groupId));
}

void GroupManager::removeMember(int groupId, int personId)
{
    if (personId == 1)
    {
        throw ExpenseError("You cannot be removed from your own group");
    }
    for (int i = 0; i < (int)groups.size(); i++)
    {
        if (groups[i].id != groupId)
        {
            continue;
        }
        for (int j = 0; j < (int)groups[i].members.size(); j++)
        {
            if (groups[i].members[j] == personId)
            {
                groups[i].members.erase(groups[i].members.begin() + j);
                return;
            }
        }
        throw ExpenseError("This person is not in the group");
    }
    throw ExpenseError("No group found with ID " + to_string(groupId));
}

void GroupManager::deleteGroup(int groupId)
{
    for (int i = 0; i < (int)groups.size(); i++)
    {
        if (groups[i].id == groupId)
        {
            groups.erase(groups.begin() + i);
            return;
        }
    }
    throw ExpenseError("No group found with ID " + to_string(groupId));
}

const Group* GroupManager::findGroup(int groupId) const
{
    for (int i = 0; i < (int)groups.size(); i++)
    {
        if (groups[i].id == groupId)
        {
            return &groups[i];
        }
    }
    return nullptr;
}

bool GroupManager::isMember(int groupId, int personId) const
{
    const Group* group = findGroup(groupId);
    if (group == nullptr)
    {
        return false;
    }
    for (int i = 0; i < (int)group->members.size(); i++)
    {
        if (group->members[i] == personId)
        {
            return true;
        }
    }
    return false;
}

void GroupManager::loadGroup(const Group& group)
{
    groups.push_back(group);
}

}
