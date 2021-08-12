#pragma once

namespace minlib
{
	static const char* CONFIG_TEMPLATE = "# *Required.  Specifies whether to use the MS Visual C++ (msvc) or GCC (g++) compiler. \r\n \
compiler = \r\n \
\r\n \
# *Required.  The file that contains all of the top-level includes. \r\n \
input_file = \r\n \
\r\n \
# MinLib will attempt to find 'vcvars32.bat' automatically, however if that fails you can specify the full path here.  This batch file is only needed when compiling using the MSVC compiler. \r\n \
msvc_vcvars32_bat = \r\n \
\r\n \
# The working directory (all relative paths will be based on this path).  Defaults to the directory from which minlib was run, but you can specify a relative path from that directory. \r\n \
working_dir = \r\n \
\r\n \
# A space-delimited list of preprocessor definitions.  Should be the same definitions normally passed when using the target library. \r\n \
defs = \r\n \
\r\n \
# A space-delimited list of additional include directories.  Defaults to the working directory. \r\n \
include_dir = \r\n \
\r\n \
# A space-delimited list of additional library directories.  Defaults to the working directory. \r\n \
lib_dir = \r\n \
\r\n \
# A space-delimited list of additional library files.  If the target library uses '#pragma comment(lib, \"some_lib\")' in the header files then you don't need to specify the libraries here. \r\n \
libs = \r\n \
\r\n \
# The directory that the extracted header files should be copied to.  Defaults to 'minlib_stage/include' within the working directory. \r\n \
include_out_dir = \r\n \
\r\n \
# The directory that the referenced libraries should be copied to.  Defaults to 'minlib_stage/lib' within the working directory. \r\n \
lib_out_dir = \r\n \
";
}