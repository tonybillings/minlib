#include "bundler.hpp"
#include <filesystem>
#include <regex>
#include "file_utils.hpp"
#include "string_utils.hpp"
#include "cli.hpp"
#include "errors.hpp"

using namespace std;
using namespace minlib;

/// <summary>
/// Ensures that the staging folders exist and are empty.
/// </summary>
/// <param name="stage_include_dir">The path to the include staging directory.</param>
/// <param name="stage_lib_dir">The path to the lib staging directory.</param>
void bundler::prepare_stage(const string& stage_include_dir, const string& stage_lib_dir)
{
	if (filesystem::exists(stage_include_dir) && filesystem::is_directory(stage_include_dir))
		filesystem::remove_all(stage_include_dir);
	filesystem::create_directory(stage_include_dir);

	if (filesystem::exists(stage_lib_dir) && filesystem::is_directory(stage_lib_dir))
		filesystem::remove_all(stage_lib_dir);
	filesystem::create_directory(stage_lib_dir);
}

/// <summary>
/// Uses the bundle to copy the header files from the target library's include directory to the include staging directory.
/// </summary>
/// <param name="bundle">The bundle produced by the compiler.</param>
/// <param name="working_dir_path">The path to the working directory.</param>
/// <param name="stage_include_dir">The path to the include staging directory.</param>
/// <param name="param_map">The parameters passed into the program.</param>
void bundler::set_stage_includes(lib_bundle& bundle, const filesystem::path& working_dir_path, const string& stage_include_dir, map<string, string> param_map)
{
	auto include_dir_param = param_map.at(cli::INCLUDE_DIR_PARAM);
	auto include_dirs = parameter::get_param_values(include_dir_param);

	for (auto& id : include_dirs)
	{
		id = get_expanded_path(id);
		if (filesystem::path(id).is_relative())
			id = (working_dir_path / id).u8string();
	}

	for (auto& include_from : bundle.include_files)
	{
		auto should_include = false;
		for (auto& id : include_dirs)
		{
			// We only want to include the header files that belong to the supplied include directories,
			// that way we avoid including system headers.
			if (size_t start_pos = include_from.find(id); start_pos == string::npos || start_pos != 0)
				continue;

			should_include = true;
			break;
		}

		if (!should_include) continue;

		auto partition = include_from.substr(0, 2);
		string include_to;

		if (regex_match(partition, regex("[A-Za-z]:")))
			include_to = (filesystem::path(stage_include_dir) / include_from.substr(3)).u8string(); // Windows
		else
			include_to = (filesystem::path(stage_include_dir) / include_from.substr(1)).u8string(); // *nix

		filesystem::create_directories(filesystem::path(include_to).parent_path());
		filesystem::copy(include_from, include_to);
	}
}

/// <summary>
/// Uses the bundle to copy the lib files from the target library's lib directory to the lib staging directory.
/// </summary>
/// <param name="bundle">The bundle produced by the compiler.</param>
/// <param name="working_dir_path">The path to the working directory.</param>
/// <param name="stage_include_dir">The path to the lib staging directory.</param>
/// <param name="param_map">The parameters passed into the program.</param>
void bundler::set_stage_libs(lib_bundle& bundle, const filesystem::path& working_dir_path, const string& stage_lib_dir, map<string, string> param_map)
{
	const auto& lib_dir_param = param_map.at(cli::LIB_DIR_PARAM);
	auto lib_dirs = parameter::get_param_values(lib_dir_param);

	for (auto& ld : lib_dirs)
	{
		ld = get_expanded_path(ld);
		if (filesystem::path(ld).is_relative())
			ld = (working_dir_path / ld).u8string();
	}

	if (map<string, string>::iterator it = param_map.find(cli::LIBS_PARAM); it != param_map.end())
	{
		auto libs = parameter::get_param_values(it->second);
		bundle.lib_files.insert(bundle.lib_files.end(), libs.begin(), libs.end());
	}

	for (auto& lib : bundle.lib_files)
	{
		for (auto& lib_dir : lib_dirs)
		{
			auto lib_from = filesystem::path(lib_dir) / lib;
			if (filesystem::exists(lib_from))
			{
				filesystem::copy(lib_from, stage_lib_dir);
				break;
			}
		}
	}
}

/// <summary>
/// Copy the header files from the include staging directory to the include target directory.
/// </summary>
/// <param name="working_dir_path">The path to the working directory.</param>
/// <param name="stage_include_dir">The path to the include staging directory.</param>
/// <param name="param_map">The parameters passed into the program.</param>
void bundler::set_target_includes(const filesystem::path& working_dir_path, const string& stage_include_dir, map<string, string> param_map)
{
	auto include_out_dir = get_expanded_path(param_map.at(cli::INCLUDE_OUT_DIR_PARAM));

	if (filesystem::path(include_out_dir).is_relative())
		include_out_dir = (filesystem::path(working_dir_path) / include_out_dir).u8string();

	if (!filesystem::exists(include_out_dir)) filesystem::create_directories(include_out_dir);
	filesystem::copy(stage_include_dir, include_out_dir, filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
}

/// <summary>
/// Copy the lib files from the lib staging directory to the lib target directory.
/// </summary>
/// <param name="working_dir_path">The path to the working directory.</param>
/// <param name="stage_include_dir">The path to the lib staging directory.</param>
/// <param name="param_map">The parameters passed into the program.</param>
void bundler::set_target_libs(const filesystem::path& working_dir_path, const string& stage_lib_dir, map<string, string> param_map)
{
	auto lib_out_dir = get_expanded_path(param_map.at(cli::LIB_OUT_DIR_PARAM));

	if (filesystem::path(lib_out_dir).is_relative())
		lib_out_dir = (filesystem::path(working_dir_path) / lib_out_dir).u8string();

	if (!filesystem::exists(lib_out_dir)) filesystem::create_directories(lib_out_dir);
	filesystem::copy(stage_lib_dir, lib_out_dir, filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
}

/// <summary>
/// Copies any additional files as specified in the 'copy_files' parameter.
/// </summary>
/// <param name="working_dir_path">The path to the working directory.</param>
/// <param name="param_map">The parameters passed into the program.</param>
void bundler::copy_files(const std::filesystem::path& working_dir_path, std::map<std::string, std::string> param_map)
{
	auto copy_operations_str = param_map.at(cli::COPY_FILES_PARAM);
	auto copy_operations_arr = str_split(copy_operations_str, ' ');

	for (auto& str : copy_operations_arr)
	{
		if (str.size() < 3)
			throw runtime_error(COPY_FILES_INVALID_ARG_ERROR);

		string s;
		string first = str.substr(0, 1);
		string last = str.substr(str.size() - 1, 1);

		if ((first == "\"" && last != "\"") || (first != "\"" && last == "\""))
			throw runtime_error(COPY_FILES_INVALID_ARG_ERROR);
		else if (first == "\"" && last == "\"")
			s = str.substr(1, str.size() - 2);
		else
			s = move(str);

		auto src_dst_arr = str_split(s, '>');
		if (src_dst_arr.size() != 2)
			throw runtime_error(COPY_FILES_INVALID_ARG_ERROR);

		auto src = get_expanded_path(src_dst_arr[0]);
		auto dst = get_expanded_path(src_dst_arr[1]);

		if (!filesystem::exists(src))
			throw runtime_error(regex_replace(COPY_FILES_SRC_MISSING_ERROR, regex("%s"), src));

		try
		{
			filesystem::create_directories(filesystem::path(dst).parent_path());
			filesystem::copy(src, dst, filesystem::copy_options::overwrite_existing);
		}
		catch (exception& ex)
		{
			string message = regex_replace(COPY_FILES_ERROR, regex("%s"), src);
			message = regex_replace(message, regex("%e"), ex.what());
			throw runtime_error(message);
		}
	}
}

/// <summary>
/// Uses the bundle to copy the header/lib files from the target library to the staging
/// directories and then subsequently to the final target directories.
/// </summary>
/// <param name="bundle">The bundle produced by the compiler.</param>
/// <param name="param_map">The parameters passed into the program.</param>
void bundler::bundle_library(lib_bundle& bundle, map<string, string> param_map)
{
	auto working_dir_path = filesystem::path(get_expanded_path(param_map.at(cli::WORKING_DIR_PARAM)));
	if (working_dir_path.is_relative())
		working_dir_path = (filesystem::current_path() / working_dir_path).u8string();

	auto stage_path = working_dir_path / filesystem::path("minlib_stage");
	auto stage_include_dir = (stage_path / "include").u8string();
	auto stage_lib_dir = (stage_path / "lib").u8string();

	prepare_stage(stage_include_dir, stage_lib_dir);

	set_stage_includes(bundle, working_dir_path, stage_include_dir, param_map);
	set_stage_libs(bundle, working_dir_path, stage_lib_dir, param_map);

	set_target_includes(working_dir_path, stage_include_dir, param_map);
	set_target_libs(working_dir_path, stage_lib_dir, param_map);

	copy_files(working_dir_path, param_map);
}
