#include "parameter.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include "string_utils.hpp"
#include "errors.hpp"

using namespace std;
using namespace minlib;

/// <summary>
/// Parameters are expected to be in the k=v format, with spaces allowed inbetween.  
/// </summary>
/// <param name="key_value">The key/value as a C-style string.</param>
/// <param name="out_param">The parameter instance that should be set.</param>
/// <returns>Returns true if the key/value was successfully parsed and the 'out-param' parameter was set.</returns>
bool parameter::parse_param(const char* key_value, parameter& out_param)
{
	if (key_value == nullptr) return false;
	auto kv = str_trim(string(key_value));
	if (kv.size() == 0) return false;
	if (kv.substr(0, 1) == string("#")) return false;

	if (kv.size() > 1 && kv.substr(kv.size() - 1, 1) == string("="))
	{
		out_param.name = kv.substr(0, kv.size() - 2);
		return true;
	}

	auto kv_arr = str_split(kv, '=');
	if (kv_arr.size() == 1)
	{
		out_param.value = str_trim(kv_arr[0]);
		return true;
	}
	else if (kv_arr.size() == 2)
	{
		out_param.name = str_trim(kv_arr[0]);
		out_param.value = str_trim(kv_arr[1]);
		return true;
	}

	return false;
}

/// <summary>
/// Get the parameters as supplied from the command-line.
/// </summary>
/// <param name="count">The number of command-line arguments passed in.</param>
/// <param name="key_value_array">The command-line arguments.</param>
/// <returns></returns>
vector<parameter> parameter::get_params(size_t count, char* key_value_array[])
{
	vector<parameter> params;
	
	for (size_t i = 0; i < count; ++i)
	{
		parameter p;
		if (parse_param(key_value_array[i], p))
			params.push_back(p);
	}

	return params;
}

/// <summary>
/// Get the parameters from the config file.
/// </summary>
/// <param name="filename">The name of the config file.</param>
/// <returns>List of parameters.</returns>
vector<parameter> parameter::get_params(const string& filename)
{
	vector<parameter> params;

	if (!filesystem::exists(filename) || !filesystem::is_regular_file(filename))
		throw runtime_error(regex_replace(CONFIG_FILE_NOT_FOUND_ERROR, regex("%s"), filename));

	ifstream config_file(filename);
	if (config_file.fail())
		throw runtime_error(regex_replace(CONFIG_FILE_READ_ERROR, regex("%s"), filename));
	
	string line;
	while (getline(config_file, line))
	{
		parameter p;
		if (parse_param(line.c_str(), p))
			params.push_back(p);
	}

	return params;
}

/// <summary>
/// Some parameters have a value that is actually a space-delimited list of values.
/// </summary>
/// <param name="value">The space-delimited list of values.</param>
/// <returns>List of values.</returns>
vector<string> parameter::get_param_values(const string& value)
{
	stringstream ss;
	ss << value;
	string val;
	vector<string> values;
	while (ss >> val) { values.push_back(val); }
	return values;
}