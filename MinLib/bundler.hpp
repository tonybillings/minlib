#pragma once

#include <map>
#include <string>
#include <filesystem>
#include "lib_bundle.hpp"

class bundler
{
private:
	static void prepare_stage(const std::string& include_dir, const std::string& lib_dir);
	static void set_stage_includes(lib_bundle& bundle, const std::filesystem::path& working_dir_path, const std::string& stage_include_dir, std::map<std::string, std::string> param_map);
	static void set_stage_libs(lib_bundle& bundle, const std::filesystem::path& working_dir_path, const std::string& stage_lib_dir, std::map<std::string, std::string> param_map);
	static void set_target_includes(const std::filesystem::path& working_dir_path, const std::string& stage_include_dir, std::map<std::string, std::string> param_map);
	static void set_target_libs(const std::filesystem::path& working_dir_path, const std::string& stage_lib_dir, std::map<std::string, std::string> param_map);

public:
	static void bundle_library(lib_bundle& bundle, std::map<std::string, std::string> param_map);
};