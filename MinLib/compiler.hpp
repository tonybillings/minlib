#pragma once

#include <map>
#include <vector>
#include <string>
#include "lib_bundle.hpp"


class compiler
{
private:
	static const char* msvc_template;
	static const char* gcc_template;

public:
	static void run_preprocessor(std::map<std::string, std::string> param_map);
	static lib_bundle parse_preprocessor_output(std::map<std::string, std::string> param_map);
};