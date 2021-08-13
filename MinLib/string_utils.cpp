#include "string_utils.hpp"
#include <algorithm> 
#include <cctype>
#include <locale>

using namespace std;

// These three functions should not have external linkage.  Only the safer
// versions that do not alter the input string are made accessible.
// This approach may be opposed in favor of using the more performant
// versions that don't make a copy of the input string.  MinLib has yet
// to be optimized...
void ltrim(string& s);
void rtrim(string& s);
void trim(string& s);


/// <summary>
/// Splits a string into a vector based on a char delimiter.
/// </summary>
/// <param name="str">The string to be split.</param>
/// <param name="delimeter">The char delimiter.</param>
/// <returns>The vector containing the delimited sub-strings.</returns>
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

/// <summary>
/// Removes all leading whitespace characters.
/// </summary>
/// <param name="str">The input string to be trimmed.</param>
/// <returns>Returns a new string instance with the leading whitespace characters removed.</returns>
string str_ltrim(const string& str)
{
    string s(str);
    ltrim(s);
    return s;
}

/// <summary>
/// Removes all trailing whitespace characters.
/// </summary>
/// <param name="str">The input string to be trimmed.</param>
/// <returns>Returns a new string instance with the trailing whitespace characters removed.</returns>
string str_rtrim(const string& str)
{
    string s(str);
    rtrim(s);
    return s;
}

/// <summary>
/// Removes all leading and trailing whitespace characters.
/// </summary>
/// <param name="str">The input string to be trimmed.</param>
/// <returns>Returns a new string instance with the leading and trailing whitespace characters removed.</returns>
string str_trim(const string& str)
{
    string s(str);
    trim(s);
    return s;
}

/// <summary>
/// Removes all leading whitespace characters.
/// </summary>
/// <param name="s">The input string to be trimmed.  The reference passed in will be modified.</param>
void ltrim(string& s)
{
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch)
        {
            return !isspace(ch);
        }));
}

/// <summary>
/// Removes all trailing whitespace characters.
/// </summary>
/// <param name="s">The input string to be trimmed.  The reference passed in will be modified.</param>
void rtrim(string& s)
{
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch)
        {
            return !isspace(ch);
        }).base(), s.end());
}

/// <summary>
/// Removes all leading and trailing whitespace characters.
/// </summary>
/// <param name="s">The input string to be trimmed.  The reference passed in will be modified.</param>
void trim(string& s)
{
    ltrim(s);
    rtrim(s);
}

