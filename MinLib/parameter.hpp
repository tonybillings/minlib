#pragma once

#include <string>
#include <vector>

class parameter
{
public:
	std::string name;
	std::string value;

public:
	static bool parse_param(const char* key_value, parameter& out_param);
	static std::vector<parameter> get_params(size_t count, char* key_value_array[]);
	static std::vector<parameter> get_params(const std::string& filename);
	static std::vector<std::string> get_param_values(const std::string& value);
};