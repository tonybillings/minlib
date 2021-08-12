#include <iostream>
#include "cli.hpp"
#include "compiler.hpp"
#include "bundler.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    try
    {
        auto params = parameter::get_params(argc - 1, argc > 1 ? &argv[1] : argv);
        auto param_map = cli::process_params(params);
        if (!cli::run_commands(param_map)) return 0;
        cout << "MinLib is running..." << endl;
        compiler::run_preprocessor(param_map);
        auto bundle = compiler::parse_preprocessor_output(param_map);
        bundler::bundle_library(bundle, param_map);
        cout << "MinLib completed successfully." << endl;
    }
    catch (exception& ex)
    {
        cout << "ERROR: " << ex.what() << endl;
    }
}