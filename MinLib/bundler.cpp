#include "bundler.hpp"
#include <filesystem>
#include <regex>
#include "file_utils.hpp"
#include "cli.hpp"

using namespace std;

void bundler::prepare_stage(const std::string& stage_include_dir, const std::string& stage_lib_dir)
{
	if (filesystem::exists(stage_include_dir) && filesystem::is_directory(stage_include_dir))
		filesystem::remove_all(stage_include_dir);
	filesystem::create_directory(stage_include_dir);

	if (filesystem::exists(stage_lib_dir) && filesystem::is_directory(stage_lib_dir))
		filesystem::remove_all(stage_lib_dir);
	filesystem::create_directory(stage_lib_dir);
}

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
			if (size_t start_pos = include_from.find(id); start_pos == string::npos || start_pos != 0)
				continue;

			should_include = true;
			break;
		}

		if (should_include)
		{
			string partition = include_from.substr(0, 2);
			string include_to;

			if (regex_match(partition, regex("[A-Za-z]:")))
				include_to = (filesystem::path(stage_include_dir) / include_from.substr(3)).u8string();
			else
				include_to = (filesystem::path(stage_include_dir) / include_from.substr(1)).u8string();

			filesystem::create_directories(filesystem::path(include_to).parent_path());
			filesystem::copy(include_from, include_to);
		}
	}
}

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

void bundler::set_target_includes(const filesystem::path& working_dir_path, const string& stage_include_dir, map<string, string> param_map)
{
	auto include_out_dir = get_expanded_path(param_map.at(cli::INCLUDE_OUT_DIR_PARAM));
	if (filesystem::path(include_out_dir).is_relative())
		include_out_dir = (filesystem::path(working_dir_path) / include_out_dir).u8string();
	if (!filesystem::exists(include_out_dir)) filesystem::create_directories(include_out_dir);
	filesystem::copy(stage_include_dir, include_out_dir, filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
}

void bundler::set_target_libs(const filesystem::path& working_dir_path, const string& stage_lib_dir, map<string, string> param_map)
{
	auto lib_out_dir = get_expanded_path(param_map.at(cli::LIB_OUT_DIR_PARAM));
	if (filesystem::path(lib_out_dir).is_relative())
		lib_out_dir = (filesystem::path(working_dir_path) / lib_out_dir).u8string();
	if (!filesystem::exists(lib_out_dir)) filesystem::create_directories(lib_out_dir);
	filesystem::copy(stage_lib_dir, lib_out_dir, filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive);
}

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
}
