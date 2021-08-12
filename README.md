# MinLib  

## Overview  

This console application can be used to extract only the absolute minimum number of required header (.h/.hpp) and library (.lib) files from another C++ project, so as to include them in your project for successful compilation.  This is particularly useful if the target project is large and you would like to share the source code of your project, along with the target project's headers/libraries, so that your project can be compiled without having the target project installed.  

For example, let's assume that your project utilizes Boost.  Normally, in order for your project to compile successfully you would need to have Boost installed locally so that you can include the required header files and link against the required libraries.  If you wanted to bundle Boost with your project so that it is self-contained (no external dependencies), it would mean adding the Boost folder that contains all of the header files and the folder containing the compiled lib files.  That is usually highly undesirable and often prohibitive, as the folder containing the header files is over 130 MB and the lib folder is over 1.2 GB (as of Boost 1.76.0).  Most projects that utilize Boost do not need access to all of the headers/libraries and that's where this tool comes in:  to figure out which headers/libraries are actually needed so that they can be extracted and bundled with your project without consuming too much storage.  The resulting size of the bundle depends on which headers your project includes, but in the case of Boost, for example, the total size of the bundled headers and libraries may be less than 10 MB.  

## Prerequisites 

This tool has been designed to run on Windows and \*nix systems.  It depends on a C++ compiler and currently works with either the Microsoft Visual C++ (MSVC) compiler or GCC.  Any compiler that allows you to invoke the preprocessor and analyze the resulting output can (theoretically) be supported.  

If using the MSVC compiler, it is recommended to install Microsoft Visual Studio with the **Desktop Development with C++** workflow option.  That will install the compiler and a batch file (vcvars32.bat) that MinLib runs within a shell environment to set up the environment variables needed to run MSVC properly.  Running that batch file is the equivalent of opening the Visual Studio Developer Command Prompt.  

For GCC, the only requirement is that GCC is installed and in your PATH.   

For convenience, a precompiled 32-bit version of MinLib is included in the project's Dist folder.  

## Compiler Installation

* [Microsoft Visual Studio](https://visualstudio.microsoft.com/downloads/)  

* [GCC for Windows](http://mingw-w64.org/doku.php)  

* [GCC for Ubuntu](https://linuxize.com/post/how-to-install-gcc-compiler-on-ubuntu-18-04/)  

* [GCC for RedHat](https://developers.redhat.com/HW/gcc-RHEL-7#2__setup_your_development_environment)  

* [GCC for CentOS](https://linuxize.com/post/how-to-install-gcc-compiler-on-centos-7/)  

## Usage  
	 
MinLib is command-line tool with no GUI and as such can be integrated within a build pipeline or executed on an ad-hoc basis.  Running MinLib requires that certain parameters be passed in either as command-line arguments or defined within a config file.  Parameters can also be passed using both methods, for example by defining parameters that don't usually change within the config file and more dynamic parameters being passed in as arguments to the executable.  
	
At a bare minimum, at least one argument is required when running MinLib, which would be the name/path of the config file containing the required parameters.  If no arguments are passed, MinLib will print usage info, similar to passing /? or -help with other commands.  To learn of all the available parameters and which are required, run this command:  

```
minlib --config
```

This is also a good way to get a config template file that you can populate and use later when running MinLib.  Simply send the output to a file instead of stdout:  

```
minlib --config > boost.ini
```
	
You can give the config file any name/extension you prefer, however the format of the file is that of typical ini files, so giving the file that extension would allow certain text editors to provide lexicon highlighting.  For the purpose of this readme, we will assume that Boost is the target library.  

The config file template indicates which parameters are required and they are included at the top of the file.  At a minimum, MinLib needs to know which compiler you want it to use and the name/path of the file that will be the input file to the compiler.  

Currently, only two values are valid for the `compiler` parameter: `msvc` and `gcc`.  For the `input_file` parameter, you can specify either a relative or absolute path, with the former being relative to the working directory (the directory from which `minlib` is run).   

The input file can have any name/extension that you prefer, however for the same reason as the config file you may want to give it the extension .h or .hpp, as it will contain **#include** statements (or not, if you intentionally prefer to make it clear that this file is not part of your project but rather part of the MinLib build process).  The #include statements in this file should be the accumulation of **all** #include statements currently found in your project and that are relative to the target library.  It should not contain anything else (comments are ok) and should be fully compilable by the chosen compiler.  Technically speaking, nothing will actually be "compiled" by the compiler, as MinLib only invokes the preprocessor for the purpose of generating one large header file, which will be parsed by MinLib to learn which include files (belonging to the target library) are needed in order to build your project.  

For example, if you use Boost only for its **thread** class, your input file may contain just the following #include statements:  

```
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/thread/future.hpp>
```

Assumming that you name this input file "boost.includes" and that you are using MSVC as the compiler, you could run MinLib with the following command:  

```
minlib compiler=msvc input_file=boost.includes
```
	
Notice that the format of the command-line arguments is a space-delimited list of key/value pairs, similar to the format of the config file.  If the value of the parameter contains spaces, encapsulate the value in double quotes:  

```
minlib compiler=msvc input_file="MinLib Input\boost.includes"
```
	
In the above example, the input file is stored in a sub-directory named "MinLib Input", relative to the current working directory.  Note that you can specify what the working directory should be by setting the `working_dir` parameter.  That will affect how all other parameters are parsed when using relative paths.  

As mentioned previously, you can also put the required parameters in a config file and then pass in the name/path to that file when running MinLib:   

Config file (boost.ini):  

```
compiler = msvc
input_file = boost.includes
```
	
MinLab command:  

```
minlib boost.ini
```
	
Notice that when passing in the argument for the config file that it is not prefixed with a parameter name.  

While the other parameters are not required, you will most likely need to set them explicitly as the default values will probably not work.  For example, by default MinLib will not pass any definitions to the compiler, but libraries like Boost and Qt expect to see certain definitions as they are used to configure the library (using preprocessor directives, they conditionally enable/disable code within the header files).  As another example, MinLib will default the `include_dir` and `lib_dir` parameters to the current working directory, which would only work if that directory happens to contain the folder/files needed to build your project using the target library.  Basically, you want to configure MinLib to use the same definitions and include/library directories as what your IDE uses when it compiles your project.  

Going back to our example use case, assume Boost is installed to **C:\Boost1_76** and that we are wanting to statically link against the Boost libraries, without debug symbols.  Your config file might then look like:  

```
compiler = msvc
input_file = boost.includes
defs = _MT _CPPUNWIND UNICODE _UNICODE WIN32 WIN64 NDEBUG Boost_USE_STATIC_LIBS
include_dir = C:\Boost1_76
lib_dir = C:\Boost1_76\stage\lib
```

Environment variables are supported in the path, using either Windows syntax (**%some_var%**) or POSIX syntax (**${some_var}**):  

```
include_dir = %BOOST_ROOT%
lib_dir = %BOOST_ROOT%\stage\lib
```

By default, MinLib will output the resulting headers/libs to the current working directory.  You can specify another target for both the header and library files:  
  
```
include_out_dir = BundledBoost\include
lib_out_dir = BundledBoost\lib
```

Again, relative and absolute paths are supported in addition to the use of environment variables.  

Once MinLib has extracted the needed files from the target library, you would then configure your IDE to use these files instead of the installed instance.  If using Visual Studio, for example, you could create a new build configuration that uses the bundled instance of the target library versus the installed one, simply by having it use different include/library paths.  