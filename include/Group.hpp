#pragma once

#include <string>
#include <vector>

#include "People.hpp"

namespace expense_tracker
{

struct Group
{
    int id = 0;
    std::string name;
    std::vector<int> members;
};

class GroupManager
{
public:
    GroupManager();

    int createGroup(const std::string& name, const std::vector<int>& members, const PeopleManager& people);
    void addMember(int groupId, int personId, const PeopleManager& people);
    void removeMember(int groupId, int personId);
    void deleteGroup(int groupId);
    const Group* findGroup(int groupId) const;
    bool isMember(int groupId, int personId) const;
    void loadGroup(const Group& group);

    const std::vector<Group>& getAll() const
    {
        return groups;
    }

    int getNextId() const
    {
        return nextId;
    }

    void setNextId(int newNextId)
    {
        nextId = newNextId;
    }

    void clearAll()
    {
        groups.clear();
    }

private:
    std::vector<Group> groups;
    int nextId;
};

}
