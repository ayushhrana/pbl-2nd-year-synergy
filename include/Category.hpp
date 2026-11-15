#pragma once

#include <string>
#include <vector>

namespace expense_tracker
{

class CategoryList
{
public:
    CategoryList();

    bool hasCategory(const std::string& name) const;
    std::string findCategory(const std::string& name) const;
    void addCategory(const std::string& name);
    std::string getCategoryAt(int position) const;

    int getCount() const
    {
        return (int)names.size();
    }

private:
    std::vector<std::string> names;
};

}
