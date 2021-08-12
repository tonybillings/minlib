#pragma once

#include <string>
#include <filesystem>

std::string get_file_contents(const char* filename);

std::string get_expanded_path(const std::string& path);

void create_directory_path(const std::filesystem::path& src, const std::filesystem::path& target);
