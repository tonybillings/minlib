#include <iostream>
#include "cli.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    try
    {
        // Retrieve and validate the input parameters.
        auto params = parameter::get_params(argc - 1, argc > 1 ? &argv[1] : argv);
        auto param_map = cli::process_params(params);
        
        // Run the command specified in the parameters.
        return cli::run_command(param_map);
    }
    catch (exception& ex)
    {
        cout << "ERROR: " << ex.what() << endl;
        return -1;
    }
}