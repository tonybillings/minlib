#include "file_utils.hpp"
#include <filesystem>
#include <regex>
#include <fstream>
#include <sstream>
#include "errors.hpp"

using namespace std;
using namespace minlib;

string get_file_contents(const char* filename) 
{
    filesystem::path filepath(filesystem::absolute(filesystem::path(filename)));
    uintmax_t file_size;

    if (filesystem::exists(filepath)) 
        file_size = filesystem::file_size(filepath);
    else
        throw runtime_error(regex_replace(FILE_NOT_FOUND, regex("%s"), filepath.string()));

    ifstream in_file;
    in_file.exceptions(ifstream::failbit | ifstream::badbit);

    try 
    {
        in_file.open(filepath.c_str(), ios::in | ifstream::binary);
    }
    catch (...) 
    {
        throw runtime_error(regex_replace(FILE_READ_ERROR, regex("%s"), filepath.string()));
    }

    string file_contents;

    try 
    {
        file_contents.resize((const unsigned int)file_size);
    }
    catch (...) 
    {
        throw runtime_error(regex_replace(STRING_RESIZE_ERROR, regex("%s"), to_string(file_size)));
    }

    in_file.read(file_contents.data(), file_size);
    in_file.close();

    return file_contents;
}

string get_expanded_path(const std::string& path)
{
    string result(path);

    auto replace_vars = [&result](const string& regex_str) {
        regex env_var(regex_str);
        std::smatch match;
        while (regex_search(result, match, env_var)) {
            auto var_name = regex_replace(match.str(0), regex("[\\%\\$\\{\\}]"), "");
            auto var_value = getenv(var_name.c_str());
            result.replace(match[0].first, match[0].second, var_value);
        }
    };

    // POSIX
    replace_vars("\\$\\{[\\w]+\\}");

    // Windows
    replace_vars("\\%[\\w]+\\%");

    return result;
}

void create_directory_path(const filesystem::path& src, const filesystem::path& target)
{
    for (const auto& dir : filesystem::recursive_directory_iterator(src.parent_path()))
    {
        const auto dir_path = dir.path();
        const auto relative_src = filesystem::relative(dir_path, src.parent_path());
        const auto target_parent_path = target / relative_src.parent_path();
        filesystem::create_directories(target_parent_path);
    }
}