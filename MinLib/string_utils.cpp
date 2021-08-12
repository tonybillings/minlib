#include "string_utils.hpp"
#include <algorithm> 
#include <cctype>
#include <locale>

using namespace std;

// These three functions should not have external linkage.
void ltrim(string& s);
void rtrim(string& s);
void trim(string& s);

vector<string> str_split(const string& str, char delimeter = ',')
{
	vector<string> result;
	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delimeter, end)) != string::npos)
	{
		end = str.find(delimeter, start);
		result.push_back(str.substr(start, end - start));
	}

    if (result.size() == 0)
        result.push_back(str);

	return result;
}

string str_ltrim(const string& str)
{
    string s(str);
    ltrim(s);
    return s;
}

string str_rtrim(const string& str)
{
    string s(str);
    rtrim(s);
    return s;
}

string str_trim(const string& str)
{
    string s(str);
    trim(s);
    return s;
}

void ltrim(string& s)
{
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch)
        {
            return !isspace(ch);
        }));
}

void rtrim(string& s)
{
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch)
        {
            return !isspace(ch);
        }).base(), s.end());
}

void trim(string& s)
{
    ltrim(s);
    rtrim(s);
}

