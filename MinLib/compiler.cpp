#include "compiler.hpp"
#include <regex>
#include <numeric>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "file_utils.hpp"
#include "string_utils.hpp"
#include "cli.hpp"

using namespace std;

// CL.exe should be run in either the Developer Command Prompt for Visual Studio
// or a regular command prompt but only after the vcvars32.bat file has been run.
const char* compiler::msvc_template = "@echo off\r\n\
call \"@msvc_vcvars32_bat@\"\r\n\
cd \"@working_dir@\\minlib_stage\"\r\n\
cl /P /MP /Fi:preprocessor_output.txt @includes@ @defs@ @input_file@\r\n\
";

const char* compiler::gcc_template = "cd \"@working_dir@\\minlib_stage\" && g++ -E -Wall -x c++ -o preprocessor_output.txt @includes@ @defs@ @input_file@";


/// <summary>
/// Use the input parameters to configure the preprocessor and have it consume the input file
/// that contains all of the #include statements used within the project that will contain
/// the bundled version of the target library.  The output of the preprocessor will be a file
/// written to disk, which will contain the combination of all header files that were included,
/// along with "line directives" that indicate the full path to the header file.  The line 
/// directives precede the content of the file they represent.
/// </summary>
/// <param name="param_map">The parameters passed into the program.</param>
void compiler::run_preprocessor(map<string, string> param_map)
{
	// Set the working directory.
	auto working_dir_path = get_expanded_path(param_map.at(cli::WORKING_DIR_PARAM));
	if (filesystem::path(working_dir_path).is_relative())
		working_dir_path = (filesystem::current_path() / filesystem::path(working_dir_path)).u8string();

	// Create the main staging directory.
	auto stage_path = filesystem::path(working_dir_path) / filesystem::path("minlib_stage");
	if (filesystem::exists(stage_path) && filesystem::is_directory(stage_path))
		filesystem::remove_all(stage_path);
	filesystem::create_directory(stage_path);

	auto get_full_path = [&](const string& path) {
		filesystem::path p(get_expanded_path(path));
		if (p.is_relative())
			return (filesystem::path(working_dir_path) / p).u8string();
		else
			return p.u8string();
	};

	auto preprocess_msvc = [&]() {
		auto bat = regex_replace(msvc_template, regex("\\@msvc_vcvars32_bat\\@"), get_expanded_path(param_map.at(cli::MSVC_BAT_PARAM)));
		bat = regex_replace(bat, regex("\\@working_dir\\@"), working_dir_path);
		bat = regex_replace(bat, regex("\\@input_file\\@"), get_full_path(param_map.at(cli::INPUT_FILE_PARAM)));

		string includes; // Additional include directories for the compiler to consider.
		if (map<string, string>::iterator it = param_map.find(cli::INCLUDE_DIR_PARAM); it != param_map.end())
		{
			auto includes_param = it->second;
			auto includes_vec = parameter::get_param_values(includes_param);
			
			for (auto& i : includes_vec)
				includes += "/I" + i + " ";
		}
		bat = regex_replace(bat, regex("\\@includes\\@"), includes);

		string defs; // Additional definitions for the compiler to define.
		if (map<string, string>::iterator it = param_map.find(cli::DEFS_PARAM); it != param_map.end())
		{
			auto defs_param = it->second;
			auto defs_vec = parameter::get_param_values(defs_param);

			for (auto& d : defs_vec)
				defs += "/D " + d + " ";
		}
		bat = regex_replace(bat, regex("\\@defs\\@"), defs);

		auto bat_path = stage_path / filesystem::path("msvc.bat");
		ofstream bat_file(bat_path);
		bat_file << bat;
		bat_file.close();

		system(bat_path.u8string().c_str());
	};

	auto preprocess_gcc = [&]() {
		auto cmd = regex_replace(gcc_template, regex("\\@working_dir\\@"), working_dir_path);
		auto input_file = param_map.at(cli::INPUT_FILE_PARAM);
		auto input_file_new_name = input_file.substr(0, input_file.find_last_of('.')) + ".cpp";
		auto input_file_path = stage_path / input_file_new_name;
		filesystem::copy(get_full_path(input_file), input_file_path);
		cmd = regex_replace(cmd, regex("\\@input_file\\@"), input_file_path.u8string());

		string includes; // Additional include directories for the compiler to consider.
		if (map<string, string>::iterator it = param_map.find(cli::INCLUDE_DIR_PARAM); it != param_map.end())
		{
			auto includes_param = it->second;
			auto includes_vec = parameter::get_param_values(includes_param);

			for (auto& i : includes_vec)
				includes += "-I" + i + " ";
		}
		cmd = regex_replace(cmd, regex("\\@includes\\@"), includes);

		string defs; // Additional definitions for the compiler to define.
		if (map<string, string>::iterator it = param_map.find(cli::DEFS_PARAM); it != param_map.end())
		{
			auto defs_param = it->second;
			auto defs_vec = parameter::get_param_values(defs_param);

			for (auto& d : defs_vec)
				defs += "-D " + d + " ";
		}
		cmd = regex_replace(cmd, regex("\\@defs\\@"), defs);

		system(cmd.c_str());
	};

	auto compiler_param = param_map.at(cli::COMPILER_PARAM);
	if (compiler_param == "msvc")
		preprocess_msvc();
	else if (compiler_param == "gcc")
		preprocess_gcc();
}

/// <summary>
/// Parses the output of the preprocessor, checking for "line directives" that indicate
/// a header file was included or that a lib file should be linked against.  All header files
/// that were included and that belong to one of the specified include directories should be
/// included as part of the bundle.  All lib files that are mentioned in the line directives
/// (via '#pragma comment') should be included in the bundle if the file is found in one of the 
/// specified lib directories.  
/// </summary>
/// <param name="param_map">The parameters passed into the program.</param>
/// <returns>Returns the list of header/lib files that should be bundled.</returns>
lib_bundle compiler::parse_preprocessor_output(map<string, string> param_map)
{
	lib_bundle result;

	auto working_dir_path = get_expanded_path(param_map.at(cli::WORKING_DIR_PARAM));
	auto stage_path = filesystem::path(working_dir_path) / filesystem::path("minlib_stage");
	auto filename = stage_path / "preprocessor_output.txt";

	ifstream file_stream(filename);
	string line;

	while (getline(file_stream, line))
	{
		line = str_trim(line);
		if (line.substr(0, 1) == "#") // Line directives begin with the pound sign.
		{
			istringstream ss(line);
			string ignored, line_num, path;
			ss >> ignored >> line_num; // Line directives have multiple parts separated by spaces.

			if (find_if(line_num.begin(), line_num.end(), [](unsigned char c) {
					return !isdigit(c); // 'include' line directives have a line number as the second part, 'lib' line directives do not.
				}) != line_num.end())
			{
				if (line.rfind("#pragma comment(lib,", 0) == 0)
				{
					auto lib_name = line.substr(20);
					lib_name = regex_replace(lib_name, regex("[\\\"\\ \\)]"), "");
					if (find(result.lib_files.begin(), result.lib_files.end(), lib_name) == result.lib_files.end())
						result.lib_files.push_back(lib_name);
				}

				continue;
			}

			ss >> path;
	
			path = regex_replace(path, regex("\\\\\\\\"), "\\");  // Replace double-slashes with single-slashes.
			path = regex_replace(path, regex("/"), "\\");  // If environment variables are used in the paths, the slashes may not be correct/consistent.
			path = regex_replace(path, regex("\\\""), ""); // Remove double-quotes.

			auto partition = path.substr(0, 2);
			string include_to;

			if (!regex_match(partition, regex("[A-Za-z]:")))
				path = regex_replace(path, regex("\\"), "/"); // Correct the slash type if on *nix.

			if (find(result.include_files.begin(), result.include_files.end(), path) == result.include_files.end())
				result.include_files.push_back(path);
		}
	}
	
	return result;
}
