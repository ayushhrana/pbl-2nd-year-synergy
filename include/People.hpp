#pragma once

#include <string>
#include <vector>

namespace expense_tracker
{

struct Person
{
    int id = 0;
    std::string name;
};

class PeopleManager
{
public:
    PeopleManager();

    int addPerson(const std::string& name);
    void renamePerson(int id, const std::string& newName);
    void removePerson(int id);
    const Person* findPerson(int id) const;
    std::string getName(int id) const;
    void loadPerson(const Person& person);

    const std::vector<Person>& getAll() const
    {
        return people;
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
        people.clear();
    }

private:
    std::string checkName(const std::string& name, int ignoreId) const;

    std::vector<Person> people;
    int nextId;
};

}
