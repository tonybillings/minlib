#pragma once

namespace minlib
{
	static const char* NO_ARGS_ERROR = "No arguments were passed.  Nothing to do.\r\n\r\nUSAGE:\r\nminlib [config_filename | --config] [key=value key=value ...]";
	static const char* CONFIG_FILE_NOT_FOUND_ERROR = "Config file '%s' not found.";
	static const char* CONFIG_FILE_READ_ERROR = "Config file '%s' could not be opened.";
	static const char* CONFIG_TEMPLATE_ARG_ERROR = "The argument '--config' cannot be combined with other arguments.";
	static const char* CONFIG_ARG_ERROR = "Only one argument, the filename of the config file, can be supplied without a name/key.  All other arguments must be passed as a key/value pair using the equal sign (k=v).";
	static const char* MISSING_ARG_ERROR = "The argument '%s' is required but was not passed to the executable or found in the config file.";
	static const char* DUPLICATE_ARG_ERROR = "The argument '%s' was supplied twice, which is not allowed.";
	static const char* MSVC_BAT_NOT_FOUND_ERROR = "Could not find the 'vcvars32.bat' batch file; please supply the path to this file via 'msvc_vcvars32_bat' parameter.";
	static const char* FILE_NOT_FOUND = "File '%s' not found.";
	static const char* FILE_READ_ERROR = "File '%s' could not be opened.";
	static const char* STRING_RESIZE_ERROR = "Could not resize string to %s bytes.";
}