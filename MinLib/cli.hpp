#pragma once

#include <vector>
#include <map>
#include "parameter.hpp"

class cli
{
private:
	static std::vector<std::string> REQUIRED_PARAMS;

public:
	static const char* CONFIG_TEMPLATE_PARAM;
	static const char* COMPILER_PARAM;
	static const char* INPUT_FILE_PARAM;
	static const char* MSVC_BAT_PARAM;
	static const char* WORKING_DIR_PARAM;
	static const char* DEFS_PARAM;
	static const char* INCLUDE_DIR_PARAM;
	static const char* LIB_DIR_PARAM;
	static const char* LIBS_PARAM;
	static const char* INCLUDE_OUT_DIR_PARAM; 
	static const char* LIB_OUT_DIR_PARAM;

private:
	static std::map<std::string, std::string> compile_params(const std::vector<parameter>& params);
	static void check_required_params(const std::map<std::string, std::string>& param_map);
	static void set_default_param_values(std::map<std::string, std::string>& param_map);

public:
	static std::map<std::string, std::string> process_params(const std::vector<parameter>& params);
	static int run_command(std::map<std::string, std::string>& param_map);
};