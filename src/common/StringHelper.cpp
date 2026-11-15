#include "StringHelper.hpp"

#include <cctype>

using namespace std;

namespace expense_tracker
{

string toLowerCase(const string& text)
{
    string result = text;
    for (int i = 0; i < (int)result.size(); i++)
    {
        result[i] = (char)tolower((unsigned char)result[i]);
    }
    return result;
}

string trimSpaces(const string& text)
{
    int start = 0;
    int end = (int)text.size() - 1;

    while (start <= end && isspace((unsigned char)text[start]))
    {
        start++;
    }
    while (end >= start && isspace((unsigned char)text[end]))
    {
        end--;
    }

    if (start > end)
    {
        return "";
    }
    return text.substr(start, end - start + 1);
}

bool isSameText(const string& first, const string& second)
{
    return toLowerCase(first) == toLowerCase(second);
}

bool containsText(const string& text, const string& part)
{
    string bigText = toLowerCase(text);
    string smallText = toLowerCase(part);
    if (bigText.find(smallText) == string::npos)
    {
        return false;
    }
    return true;
}

bool isOnlyDigits(const string& text)
{
    if (text.empty())
    {
        return false;
    }
    for (int i = 0; i < (int)text.size(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return false;
        }
    }
    return true;
}

// makes a text safe to keep in one field of a line of a txt file
string encodeField(const string& text)
{
    string result = "";
    for (int i = 0; i < (int)text.size(); i++)
    {
        if (text[i] == '%')
        {
            result = result + "%25";
        }
        else if (text[i] == '|')
        {
            result = result + "%7C";
        }
        else if (text[i] == '\n')
        {
            result = result + "%0A";
        }
        else if (text[i] == '\r')
        {
            result = result + "%0D";
        }
        else
        {
            result = result + text[i];
        }
    }
    return result;
}

string decodeField(const string& text)
{
    string result = "";
    int i = 0;
    while (i < (int)text.size())
    {
        string code = "";
        if (text[i] == '%' && i + 2 < (int)text.size())
        {
            code = text.substr(i, 3);
        }

        if (code == "%25")
        {
            result = result + "%";
            i = i + 3;
        }
        else if (code == "%7C")
        {
            result = result + "|";
            i = i + 3;
        }
        else if (code == "%0A")
        {
            result = result + "\n";
            i = i + 3;
        }
        else if (code == "%0D")
        {
            result = result + "\r";
            i = i + 3;
        }
        else
        {
            result = result + text[i];
            i = i + 1;
        }
    }
    return result;
}

vector<string> splitText(const string& text, char separator)
{
    vector<string> parts;
    string current = "";
    for (int i = 0; i < (int)text.size(); i++)
    {
        if (text[i] == separator)
        {
            parts.push_back(current);
            current = "";
        }
        else
        {
            current = current + text[i];
        }
    }
    parts.push_back(current);
    return parts;
}

}
