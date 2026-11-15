#pragma once

#include <string>
#include <vector>

namespace expense_tracker
{

enum JsonType
{
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

class JsonValue
{
public:
    JsonValue();

    static JsonValue makeBool(bool value);
    static JsonValue makeNumber(long long value);
    static JsonValue makeString(const std::string& value);
    static JsonValue makeArray();
    static JsonValue makeObject();
    static JsonValue parse(const std::string& text);

    JsonType getType() const
    {
        return type;
    }

    bool asBool() const;
    long long asNumber() const;
    std::string asString() const;

    int size() const;
    const JsonValue& at(int index) const;
    void add(const JsonValue& value);

    bool has(const std::string& key) const;
    const JsonValue& get(const std::string& key) const;
    void set(const std::string& key, const JsonValue& value);
    void setString(const std::string& key, const std::string& value);
    void setNumber(const std::string& key, long long value);
    void setBool(const std::string& key, bool value);

    std::string getString(const std::string& key) const;
    std::string getStringOr(const std::string& key, const std::string& fallback) const;
    long long getNumber(const std::string& key) const;
    long long getNumberOr(const std::string& key, long long fallback) const;
    bool getBoolOr(const std::string& key, bool fallback) const;

    std::string toString() const;

private:
    JsonType type;
    bool boolValue;
    long long numberValue;
    std::string stringValue;
    std::vector<JsonValue> items;
    std::vector<std::string> keys;
    std::vector<JsonValue> values;
};

}
