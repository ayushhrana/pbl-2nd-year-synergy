#include "People.hpp"

#include "Errors.hpp"
#include "StringHelper.hpp"

using namespace std;

namespace expense_tracker
{

PeopleManager::PeopleManager()
{
    nextId = 1;
    addPerson("You");
}

string PeopleManager::checkName(const string& name, int ignoreId) const
{
    string cleanName = trimSpaces(name);
    if (cleanName.empty())
    {
        throw ExpenseError("Name cannot be empty");
    }
    if (cleanName.size() > 40)
    {
        throw ExpenseError("Name is too long (maximum 40 letters)");
    }
    for (int i = 0; i < (int)people.size(); i++)
    {
        if (people[i].id != ignoreId && isSameText(people[i].name, cleanName))
        {
            throw ExpenseError("A person named " + cleanName + " already exists");
        }
    }
    return cleanName;
}

int PeopleManager::addPerson(const string& name)
{
    Person person;
    person.id = nextId;
    person.name = checkName(name, 0);
    people.push_back(person);
    nextId = nextId + 1;
    return person.id;
}

void PeopleManager::renamePerson(int id, const string& newName)
{
    for (int i = 0; i < (int)people.size(); i++)
    {
        if (people[i].id == id)
        {
            people[i].name = checkName(newName, id);
            return;
        }
    }
    throw ExpenseError("No person found with ID " + to_string(id));
}

void PeopleManager::removePerson(int id)
{
    if (id == 1)
    {
        throw ExpenseError("You cannot remove yourself");
    }
    for (int i = 0; i < (int)people.size(); i++)
    {
        if (people[i].id == id)
        {
            people.erase(people.begin() + i);
            return;
        }
    }
    throw ExpenseError("No person found with ID " + to_string(id));
}

const Person* PeopleManager::findPerson(int id) const
{
    for (int i = 0; i < (int)people.size(); i++)
    {
        if (people[i].id == id)
        {
            return &people[i];
        }
    }
    return nullptr;
}

string PeopleManager::getName(int id) const
{
    const Person* person = findPerson(id);
    if (person == nullptr)
    {
        return "Unknown";
    }
    return person->name;
}

void PeopleManager::loadPerson(const Person& person)
{
    people.push_back(person);
}

}
