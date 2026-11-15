#pragma once

#include <string>

#include "Date.hpp"

namespace expense_tracker
{

struct SettlementData
{
    int groupId = 0;
    int fromId = 0;
    int toId = 0;
    long long amount = 0;
    Date date;
    std::string note;
};

class Settlement
{
public:
    Settlement(int id, const SettlementData& data) : id(id), data(data), deleted(false)
    {
    }

    int getId() const
    {
        return id;
    }

    const SettlementData& getData() const
    {
        return data;
    }

    bool isDeleted() const
    {
        return deleted;
    }

    void setDeleted(bool value)
    {
        deleted = value;
    }

private:
    int id;
    SettlementData data;
    bool deleted;
};

}
