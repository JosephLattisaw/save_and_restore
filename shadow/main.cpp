#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include "shadow.hpp"

namespace {
const std::string OPTIONS_DESCRIPTION = "Options";
const std::string APPLICATION_VERSION = "Shadow 1.0.0";
const std::string APPLICATION_DESCRIPTION = "Service that controls Save and Restore Applications and is a backend to a Save/Restore GUI";

// Creating an options table for user experience
const int OPTIONS_NUMBER_PARAMS = 3;
const int OPTIONS_NUMBER_ELEMENTS = 5;
const std::array<const std::array<std::string, OPTIONS_NUMBER_PARAMS>, OPTIONS_NUMBER_ELEMENTS> OPTIONS_HANDLE = {{
    {"help", "h", "Displays this help."},
    {"version", "v", "Displays version information"},
    {"script_path", "", "Path to Scripts of Applications to be Executed"},
    {"application_names", "", "Names of Applications"},
    {"always_host_only", "", "Processes that are always Host Only"},
}};

// enumeration of options
enum OPTIONS {
    HELP = 0,
    VERSION = 1,
    SCRIPT_PATH = 2,
    APP_NAMES = 3,
    ALWAYS_HOST_ONLY = 4,
};

// enumeration of option parameters
enum OPTION_HANDLES {
    HANDLE = 0,
    SHORT_HANDLE = 1,
    DESCRIPTION = 2,
};

// utility function to print version number
void print_version_number() { std::cout << APPLICATION_VERSION << std::endl; }

// utility function to get a specific options handle based on options table
std::string get_option_handle(OPTIONS options_index, OPTION_HANDLES handle_index) {
    std::string res;
    try {
        auto opt_h = OPTIONS_HANDLE.at(options_index);
        res = opt_h.at(handle_index);
    } catch (std::out_of_range const &exc) {
        std::cout << __PRETTY_FUNCTION__ << ", options_index: " << options_index << ", handle_index: " << handle_index << std::endl;
        throw;
    }

    return res;
}

// utility function to get a specific options handle
std::string get_option_handles(OPTIONS index) {
    return get_option_handle(index, OPTION_HANDLES::HANDLE) + "," + get_option_handle(index, OPTION_HANDLES::SHORT_HANDLE);
}

// utility function to get the options description
std::string get_options_description(OPTIONS index) { return get_option_handle(index, OPTION_HANDLES::DESCRIPTION); }

// utility function to get the options handle
std::string get_options_long_handle(OPTIONS index) { return get_option_handle(index, OPTION_HANDLES::HANDLE); }
}  // namespace

int main(int argc, char **argv) {
    namespace prog_opts = boost::program_options;

    std::vector<std::string> application_names;
    std::vector<std::string> script_paths;
    std::vector<bool> always_host_only;

    // Getting our options values. NOTE: created a separate object only for readability
    auto hlp_hdl = get_option_handles(OPTIONS::HELP);
    auto hlp_desc = get_options_description(OPTIONS::HELP);

    auto opt_hdl = get_option_handles(OPTIONS::VERSION);
    auto opt_desc = get_options_description(OPTIONS::VERSION);

    auto app_hdl = get_option_handles(OPTIONS::SCRIPT_PATH);
    auto app_opt = prog_opts::value<decltype(script_paths)>(&script_paths)->multitoken();
    auto app_desc = get_options_description(OPTIONS::SCRIPT_PATH);

    auto aho_hdl = get_option_handles(OPTIONS::ALWAYS_HOST_ONLY);
    auto aho_opt = prog_opts::value<decltype(always_host_only)>(&always_host_only)->multitoken();
    auto aho_desc = get_options_description(OPTIONS::ALWAYS_HOST_ONLY);

    auto name_hdl = get_option_handles(OPTIONS::APP_NAMES);
    auto name = prog_opts::value<decltype(application_names)>(&application_names)->multitoken();
    auto name_desc = get_options_description(OPTIONS::APP_NAMES);

    // creating our options table
    prog_opts::options_description desc(OPTIONS_DESCRIPTION);
    desc.add_options()(hlp_hdl.c_str(), hlp_desc.c_str())(
        opt_hdl.c_str(), opt_desc.c_str())(app_hdl.c_str(), app_opt, app_desc.c_str())(aho_hdl.c_str(), aho_opt,
                                                                                       aho_desc.c_str())(name_hdl.c_str(), name, name_desc.c_str());

    // grabbing options from command line
    prog_opts::variables_map vars_map;
    prog_opts::store(prog_opts::parse_command_line(argc, argv, desc), vars_map);
    prog_opts::notify(vars_map);

    // if user selected help option
    if (vars_map.count(get_options_long_handle(OPTIONS::HELP))) {
        print_version_number();
        std::cout << APPLICATION_DESCRIPTION << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }
    // if user selected version option
    else if (vars_map.count(get_options_long_handle(OPTIONS::VERSION))) {
        print_version_number();
        return 0;
    }

    if (script_paths.size() == 0) {
        std::cerr << "Usage: Must specify at least one script path" << std::endl;
        std::exit(EXIT_FAILURE);
    } else if (script_paths.size() != always_host_only.size()) {
        std::cerr << "Usage: script_paths and always_host_only should be the same size: " << script_paths.size() << "," << always_host_only.size()
                  << std::endl;
        std::exit(EXIT_FAILURE);
    } else if (script_paths.size() != application_names.size()) {
        std::cerr << "Usage: script_paths and application_names should be the same size: " << script_paths.size() << "," << application_names.size()
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    boost::asio::io_service io_service;

    Shadow shadow(application_names, script_paths, always_host_only, io_service);

    io_service.run();
}
