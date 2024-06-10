#include "util/util.h"
#include <unistd.h>
#include <curl/curl.h>
#include "llama/llama.h"
#include "ui/Console.h"
#include "pollbot/StaticResponse.h"
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

bool debug_mode;

void handle_args(int argc, char** argv) {
    po::options_description description("Usage");
    description.add_options()
    ("help,h", "output help message")
    ("debug,d", po::bool_switch(&debug_mode)->default_value(false), "show debug messages");

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(description).run(), vm);

        if(vm.count("help")) {
            std::cout << description << "\n";
            exit(0);
        }
        po::notify(vm);

    } catch(po::unknown_option& e) {
        std::cout << "Invalid option \"" << e.get_option_name() << "\". Use --help to get all available options.\n";
        exit(0);
    } catch(po::invalid_option_value& e) {
        std::cout << "Invalid option value for " << e.get_option_name() << ".\n";
        exit(0);
    } catch(po::required_option& e) {
        std::cout << description << "\n";
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    if(!isatty(STDOUT_FILENO)) return 0;
    handle_args(argc, argv);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    Pollbot::Console console;

#ifndef NDEBUG
    debug_mode = true;
#endif
    if(debug_mode) {
        get_logger().logger.get()->set_level(spdlog::level::debug);
        DEBUG("Debug messages are visible.");
    }

    StaticResponse::load_static();
    load_site_blacklist();

    console.load_accounts();
    console.run();
    curl_global_cleanup();

    return 0;
}

/*
    mResponse response = client.take_screenshot().get();
    std::ofstream ostream("screenshot");
    ostream << to_string(response.body["value"]).substr(1, to_string(response.body["value"]).length()-2);
    ostream.close();
    popen("base64 --decode screenshot > a.png", "r");
*/
