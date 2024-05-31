#include "util/util.h"
#include <unistd.h>
#include <curl/curl.h>
#include "llama/llama.h"
#include "ui/Console.h"
#include "pollbot/StaticResponse.h"

void signal_handler(int sig) {
    // endwin();
    exit(0);
}

int main() {
    if(!isatty(STDOUT_FILENO)) return 0;
    // set_signal_handlers(signal_handler);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    // initscr();

    Pollbot::Console console;
    get_logger().logger.get()->set_level(spdlog::level::debug);

    StaticResponse::load_static();

    console.load_accounts();
    console.instances[0]->launch();
    console.run();
    // endwin();
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
