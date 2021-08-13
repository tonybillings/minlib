#include "cli.hpp"
#include <iostream>
#include <stdexcept>
#include <regex>
#include <filesystem>
#include "errors.hpp"
#include "config_template.hpp"
#include "compiler.hpp"
#include "bundler.hpp"

using namespace std;
using namespace minlib;

vector<string> cli::REQUIRED_PARAMS{ "compiler", "input_file" };
const char* cli::CONFIG_TEMPLATE_PARAM = "__config_template";
const char* cli::COMPILER_PARAM = "compiler";
const char* cli::INPUT_FILE_PARAM = "input_file";
const char* cli::MSVC_BAT_PARAM = "msvc_vcvars32_bat";
const char* cli::WORKING_DIR_PARAM = "working_dir";
const char* cli::DEFS_PARAM = "defs";
const char* cli::INCLUDE_DIR_PARAM = "include_dir";
const char* cli::LIB_DIR_PARAM = "lib_dir";
const char* cli::LIBS_PARAM = "libs";
const char* cli::INCLUDE_OUT_DIR_PARAM = "include_out_dir";
const char* cli::LIB_OUT_DIR_PARAM = "lib_out_dir";

map<string, string> cli::compile_params(const vector<parameter>& params)
{
	map<string, string> param_map;

	auto cfg_arg_found = false;
	auto cfg_template_arg_found = false;

	auto add_param = [&](const parameter& p) {
		auto it = param_map.find(p.name);

		if (it != param_map.end())
			throw runtime_error(regex_replace(DUPLICATE_ARG_ERROR, regex("%s"), p.name));
		else
			param_map.insert({ p.name, p.value });
	};

	for (auto& p : params)
	{
		if (p.name == string("")) // Some parameters are not named.
		{
			if (p.value == string("--config"))
			{
				if (cfg_template_arg_found) // --config should be passed in once.
				{
					throw runtime_error(regex_replace(DUPLICATE_ARG_ERROR, regex("%s"), p.value));
				}
				else if (param_map.size() > 0)  // --config should be passed in by itself.
				{
					throw runtime_error(CONFIG_TEMPLATE_ARG_ERROR);
				}
				else
				{
					cfg_template_arg_found = true;
					param_map.insert({ string("__config_template"), p.value });
				}
			}
			else  
			{
				if (cfg_arg_found) // Only the config file parameter is passed in without a name.
				{
					throw runtime_error(CONFIG_ARG_ERROR);
				}
				else if (cfg_template_arg_found)  // --config should be passed in by itself.
				{
					throw runtime_error(CONFIG_TEMPLATE_ARG_ERROR);
				}
				else
				{
					cfg_arg_found = true;
					param_map.insert({ string("__config"), p.value });
				}
			}
		}
		else
		{
			if (cfg_template_arg_found)  // --config should be passed in by itself.
				throw runtime_error(CONFIG_TEMPLATE_ARG_ERROR);
			else
				add_param(p);
		}
	}

	if (!cfg_template_arg_found && cfg_arg_found)
	{
		auto config_filename = param_map.at("__config");
		auto config_file_params = parameter::get_params(config_filename);

		for (auto& p : config_file_params)
			add_param(p);
	}

	return param_map;
}

void cli::check_required_params(const map<string, string>& param_map)
{
	// If the user passed in '--config' as a command-line arg, then no
	// other parameters are required.
	if (param_map.find("__config_template") != param_map.end())
		return;

	for (auto& rp : REQUIRED_PARAMS)
	{
		if (param_map.find(rp) == param_map.end())
			throw runtime_error(regex_replace(MISSING_ARG_ERROR, regex("%s"), rp));
	}
}

void cli::set_default_param_values(map<string, string>& param_map)
{
	auto set_param = [&param_map](map<string, string>::iterator it, string name, string value) {
		if (it == param_map.end())
			param_map.insert({ name, value });
		else
			it->second = value;
	};

	// Try to find the vcvars32.bat file, if the path to it wasn't specified using the 'msvc_vcvars32_bat' parameter.
	auto set_msvc_vcvars32_bat = [&param_map, &set_param]() {
		string param_name(MSVC_BAT_PARAM);
		if (map<string, string>::iterator it = param_map.find(param_name); it == param_map.end() || it->second == "")
		{
			string path_template("C:\\Program Files (x86)\\Microsoft Visual Studio\\@version@\\@edition@\\VC\\Auxiliary\\Build\\vcvars32.bat");
			string path;

			path = regex_replace(path_template, regex("\\@version\\@"), "2019");
			path = regex_replace(path, regex("\\@edition\\@"), "Professional");
			if (filesystem::exists(path)) { set_param(it, param_name, path); return; }

			path = regex_replace(path_template, regex("\\@version\\@"), "2019");
			path = regex_replace(path, regex("\\@edition\\@"), "Community");
			if (filesystem::exists(path)) { set_param(it, param_name, path); return; }

			path = regex_replace(path_template, regex("\\@version\\@"), "2017");
			path = regex_replace(path, regex("\\@edition\\@"), "Professional");
			if (filesystem::exists(path)) { set_param(it, param_name, path); return; }

			path = regex_replace(path_template, regex("\\@version\\@"), "2017");
			path = regex_replace(path, regex("\\@edition\\@"), "Community");
			if (filesystem::exists(path)) { set_param(it, param_name, path); return; }

			throw runtime_error(MSVC_BAT_NOT_FOUND_ERROR);
		}
	};

	auto set_working_dir = [&param_map, &set_param]() {
		string param_name(WORKING_DIR_PARAM);
		if (map<string, string>::iterator it = param_map.find(param_name); it == param_map.end() || it->second == "")
			set_param(it, param_name, filesystem::current_path().u8string());
	};

	auto set_include_dir = [&param_map, &set_param]() {
		string param_name(INCLUDE_DIR_PARAM);
		if (map<string, string>::iterator it = param_map.find(param_name); it == param_map.end() || it->second == "")
			set_param(it, param_name, filesystem::current_path().u8string());
	};

	auto set_lib_dir = [&param_map, &set_param]() {
		string param_name(LIB_DIR_PARAM);
		if (map<string, string>::iterator it = param_map.find(param_name); it == param_map.end() || it->second == "")
			set_param(it, param_name, filesystem::current_path().u8string());
	};

	auto set_include_out_dir = [&param_map, &set_param]() {
		string param_name(INCLUDE_OUT_DIR_PARAM);
		if (map<string, string>::iterator it = param_map.find(param_name); it == param_map.end() || it->second == "")
			set_param(it, param_name, (filesystem::current_path() / "include").u8string());
	};

	auto set_lib_out_dir = [&param_map, &set_param]() {
		string param_name(LIB_OUT_DIR_PARAM);
		if (map<string, string>::iterator it = param_map.find(param_name); it == param_map.end() || it->second == "")
			set_param(it, param_name, (filesystem::current_path() / "lib").u8string());
	};

	// Ensure that the vcvars32.bat file is found if MSVC was the chosen compiler.
	auto compiler_param = param_map.at(cli::COMPILER_PARAM);
	if (compiler_param == "msvc")
		set_msvc_vcvars32_bat();

	set_working_dir();
	set_include_dir();
	set_lib_dir();
	set_include_out_dir();
	set_lib_out_dir();
}

map<string, string> cli::process_params(const vector<parameter>& params)
{
	if (params.size() == 0)
		throw runtime_error(NO_ARGS_ERROR);

	// Combine the parameters specified in the config file
	// and/or those passed in as command-line arguments.
	auto param_map = compile_params(params);

	// Ensure the parameters that are required were set.
	check_required_params(param_map);

	// For all optional parameters that were not set, assign 
	// them the default value.
	set_default_param_values(param_map);

	return param_map;
}

int cli::run_command(map<string, string>& param_map)
{
	if (map<string, string>::iterator it = param_map.find(cli::CONFIG_TEMPLATE_PARAM); it != param_map.end())
	{
		// Print the config template to stdout, then exit the program.
		cout << CONFIG_TEMPLATE << endl;
		return 0;
	}

	cout << "MinLib is running..." << endl;

	// Invoke the compiler's preprocessor to have it evaluate all
	// the specified header files listed in the input file.
	compiler::run_preprocessor(param_map);

	// Comb through the output of the preprocessor, building a 
	// list of all the header and lib files to be bundled.
	auto bundle = compiler::parse_preprocessor_output(param_map);

	// Create the bundle and save to the specified output directories.
	bundler::bundle_library(bundle, param_map);

	cout << "MinLib completed successfully." << endl;

	return 0;
}
