#include "Json.hpp"

#include "Errors.hpp"

using namespace std;

namespace expense_tracker
{

JsonValue::JsonValue()
{
    type = JSON_NULL;
    boolValue = false;
    numberValue = 0;
}

JsonValue JsonValue::makeBool(bool value)
{
    JsonValue result;
    result.type = JSON_BOOL;
    result.boolValue = value;
    return result;
}

JsonValue JsonValue::makeNumber(long long value)
{
    JsonValue result;
    result.type = JSON_NUMBER;
    result.numberValue = value;
    return result;
}

JsonValue JsonValue::makeString(const string& value)
{
    JsonValue result;
    result.type = JSON_STRING;
    result.stringValue = value;
    return result;
}

JsonValue JsonValue::makeArray()
{
    JsonValue result;
    result.type = JSON_ARRAY;
    return result;
}

JsonValue JsonValue::makeObject()
{
    JsonValue result;
    result.type = JSON_OBJECT;
    return result;
}

bool JsonValue::asBool() const
{
    if (type != JSON_BOOL)
    {
        throw ExpenseError("Expected true or false");
    }
    return boolValue;
}

long long JsonValue::asNumber() const
{
    if (type != JSON_NUMBER)
    {
        throw ExpenseError("Expected a number");
    }
    return numberValue;
}

string JsonValue::asString() const
{
    if (type != JSON_STRING)
    {
        throw ExpenseError("Expected text");
    }
    return stringValue;
}

int JsonValue::size() const
{
    if (type == JSON_ARRAY)
    {
        return (int)items.size();
    }
    if (type == JSON_OBJECT)
    {
        return (int)keys.size();
    }
    return 0;
}

const JsonValue& JsonValue::at(int index) const
{
    if (type != JSON_ARRAY || index < 0 || index >= (int)items.size())
    {
        throw ExpenseError("Wrong array position");
    }
    return items[index];
}

void JsonValue::add(const JsonValue& value)
{
    if (type != JSON_ARRAY)
    {
        throw ExpenseError("Not an array");
    }
    items.push_back(value);
}

bool JsonValue::has(const string& key) const
{
    for (int i = 0; i < (int)keys.size(); i++)
    {
        if (keys[i] == key)
        {
            return true;
        }
    }
    return false;
}

const JsonValue& JsonValue::get(const string& key) const
{
    for (int i = 0; i < (int)keys.size(); i++)
    {
        if (keys[i] == key)
        {
            return values[i];
        }
    }
    throw ExpenseError("Missing field: " + key);
}

void JsonValue::set(const string& key, const JsonValue& value)
{
    if (type != JSON_OBJECT)
    {
        throw ExpenseError("Not an object");
    }
    for (int i = 0; i < (int)keys.size(); i++)
    {
        if (keys[i] == key)
        {
            values[i] = value;
            return;
        }
    }
    keys.push_back(key);
    values.push_back(value);
}

void JsonValue::setString(const string& key, const string& value)
{
    set(key, makeString(value));
}

void JsonValue::setNumber(const string& key, long long value)
{
    set(key, makeNumber(value));
}

void JsonValue::setBool(const string& key, bool value)
{
    set(key, makeBool(value));
}

string JsonValue::getString(const string& key) const
{
    return get(key).asString();
}

string JsonValue::getStringOr(const string& key, const string& fallback) const
{
    if (!has(key) || get(key).getType() == JSON_NULL)
    {
        return fallback;
    }
    return get(key).asString();
}

long long JsonValue::getNumber(const string& key) const
{
    return get(key).asNumber();
}

long long JsonValue::getNumberOr(const string& key, long long fallback) const
{
    if (!has(key) || get(key).getType() == JSON_NULL)
    {
        return fallback;
    }
    return get(key).asNumber();
}

bool JsonValue::getBoolOr(const string& key, bool fallback) const
{
    if (!has(key) || get(key).getType() == JSON_NULL)
    {
        return fallback;
    }
    return get(key).asBool();
}

static string quoteText(const string& text)
{
    string result = "\"";
    for (int i = 0; i < (int)text.size(); i++)
    {
        unsigned char c = (unsigned char)text[i];
        if (c == '"')
        {
            result += "\\\"";
        }
        else if (c == '\\')
        {
            result += "\\\\";
        }
        else if (c == '\n')
        {
            result += "\\n";
        }
        else if (c == '\r')
        {
            result += "\\r";
        }
        else if (c == '\t')
        {
            result += "\\t";
        }
        else if (c < 32)
        {
            const char* digits = "0123456789abcdef";
            result += "\\u00";
            result += digits[c / 16];
            result += digits[c % 16];
        }
        else
        {
            result += (char)c;
        }
    }
    result += "\"";
    return result;
}

string JsonValue::toString() const
{
    if (type == JSON_NULL)
    {
        return "null";
    }
    if (type == JSON_BOOL)
    {
        if (boolValue)
        {
            return "true";
        }
        return "false";
    }
    if (type == JSON_NUMBER)
    {
        return to_string(numberValue);
    }
    if (type == JSON_STRING)
    {
        return quoteText(stringValue);
    }
    if (type == JSON_ARRAY)
    {
        string result = "[";
        for (int i = 0; i < (int)items.size(); i++)
        {
            if (i > 0)
            {
                result += ",";
            }
            result += items[i].toString();
        }
        return result + "]";
    }

    string result = "{";
    for (int i = 0; i < (int)keys.size(); i++)
    {
        if (i > 0)
        {
            result += ",";
        }
        result += quoteText(keys[i]) + ":" + values[i].toString();
    }
    return result + "}";
}

class JsonParser
{
public:
    JsonParser(const string& text) : text(text), position(0), depth(0)
    {
    }

    JsonValue parseDocument()
    {
        JsonValue result = parseValue();
        skipSpaces();
        if (position != text.size())
        {
            fail("extra characters at the end");
        }
        return result;
    }

private:
    void fail(const string& message)
    {
        throw ExpenseError("Invalid JSON: " + message);
    }

    void skipSpaces()
    {
        while (position < text.size() && (text[position] == ' ' || text[position] == '\n' ||
                                          text[position] == '\r' || text[position] == '\t'))
        {
            position++;
        }
    }

    char peek()
    {
        if (position >= text.size())
        {
            fail("unexpected end");
        }
        return text[position];
    }

    void expect(char wanted)
    {
        if (peek() != wanted)
        {
            fail(string("expected ") + wanted);
        }
        position++;
    }

    bool matchWord(const string& word)
    {
        if (text.compare(position, word.size(), word) == 0)
        {
            position += word.size();
            return true;
        }
        return false;
    }

    JsonValue parseValue()
    {
        skipSpaces();
        char c = peek();
        if (c == '{')
        {
            return parseObject();
        }
        if (c == '[')
        {
            return parseArray();
        }
        if (c == '"')
        {
            return JsonValue::makeString(parseString());
        }
        if (c == '-' || (c >= '0' && c <= '9'))
        {
            return parseNumber();
        }
        if (matchWord("true"))
        {
            return JsonValue::makeBool(true);
        }
        if (matchWord("false"))
        {
            return JsonValue::makeBool(false);
        }
        if (matchWord("null"))
        {
            return JsonValue();
        }
        fail("unknown value");
        return JsonValue();
    }

    JsonValue parseObject()
    {
        depth++;
        if (depth > 40)
        {
            fail("too deep");
        }
        JsonValue result = JsonValue::makeObject();
        expect('{');
        skipSpaces();
        if (peek() == '}')
        {
            position++;
            depth--;
            return result;
        }
        while (true)
        {
            skipSpaces();
            string key = parseString();
            skipSpaces();
            expect(':');
            result.set(key, parseValue());
            skipSpaces();
            if (peek() == ',')
            {
                position++;
            }
            else
            {
                expect('}');
                break;
            }
        }
        depth--;
        return result;
    }

    JsonValue parseArray()
    {
        depth++;
        if (depth > 40)
        {
            fail("too deep");
        }
        JsonValue result = JsonValue::makeArray();
        expect('[');
        skipSpaces();
        if (peek() == ']')
        {
            position++;
            depth--;
            return result;
        }
        while (true)
        {
            result.add(parseValue());
            skipSpaces();
            if (peek() == ',')
            {
                position++;
            }
            else
            {
                expect(']');
                break;
            }
        }
        depth--;
        return result;
    }

    unsigned long readHex4()
    {
        if (position + 4 > text.size())
        {
            fail("bad \\u code");
        }
        unsigned long code = 0;
        for (int i = 0; i < 4; i++)
        {
            char c = text[position++];
            code = code * 16;
            if (c >= '0' && c <= '9')
            {
                code += c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                code += c - 'a' + 10;
            }
            else if (c >= 'A' && c <= 'F')
            {
                code += c - 'A' + 10;
            }
            else
            {
                fail("bad \\u code");
            }
        }
        return code;
    }

    void addUtf8(string& result, unsigned long code)
    {
        if (code < 0x80)
        {
            result += (char)code;
        }
        else if (code < 0x800)
        {
            result += (char)(0xC0 | (code >> 6));
            result += (char)(0x80 | (code & 0x3F));
        }
        else if (code < 0x10000)
        {
            result += (char)(0xE0 | (code >> 12));
            result += (char)(0x80 | ((code >> 6) & 0x3F));
            result += (char)(0x80 | (code & 0x3F));
        }
        else
        {
            result += (char)(0xF0 | (code >> 18));
            result += (char)(0x80 | ((code >> 12) & 0x3F));
            result += (char)(0x80 | ((code >> 6) & 0x3F));
            result += (char)(0x80 | (code & 0x3F));
        }
    }

    string parseString()
    {
        expect('"');
        string result;
        while (true)
        {
            if (position >= text.size())
            {
                fail("text is not closed");
            }
            char c = text[position++];
            if (c == '"')
            {
                break;
            }
            if (c != '\\')
            {
                result += c;
                continue;
            }
            char escape = peek();
            position++;
            if (escape == 'n')
            {
                result += '\n';
            }
            else if (escape == 't')
            {
                result += '\t';
            }
            else if (escape == 'r')
            {
                result += '\r';
            }
            else if (escape == 'b')
            {
                result += '\b';
            }
            else if (escape == 'f')
            {
                result += '\f';
            }
            else if (escape == '"' || escape == '\\' || escape == '/')
            {
                result += escape;
            }
            else if (escape == 'u')
            {
                unsigned long code = readHex4();
                if (code >= 0xD800 && code <= 0xDBFF && text.compare(position, 2, "\\u") == 0)
                {
                    position += 2;
                    unsigned long low = readHex4();
                    code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                }
                addUtf8(result, code);
            }
            else
            {
                fail("bad escape");
            }
        }
        return result;
    }

    JsonValue parseNumber()
    {
        size_t start = position;
        if (text[position] == '-')
        {
            position++;
        }
        while (position < text.size() && text[position] >= '0' && text[position] <= '9')
        {
            position++;
        }
        if (position < text.size() && (text[position] == '.' || text[position] == 'e' || text[position] == 'E'))
        {
            fail("only whole numbers are allowed");
        }
        string digits = text.substr(start, position - start);
        if (digits == "-" || digits.size() > 15)
        {
            fail("bad number");
        }
        return JsonValue::makeNumber(stoll(digits));
    }

    string text;
    size_t position;
    int depth;
};

JsonValue JsonValue::parse(const string& text)
{
    JsonParser parser(text);
    return parser.parseDocument();
}

}
