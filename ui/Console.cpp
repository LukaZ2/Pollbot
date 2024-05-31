#include "Console.h"
#include <fstream>
#include "../util/util.h"
#include <memory>
#include <readline/readline.h>
#include "command/ConsoleCommand.h"

bool Pollbot::Console::load_accounts() {
    std::string raw;
    {
        std::ifstream file("accounts.json");
        assert_em(file.is_open(), "Failed to open accounts.json");
        std::stringstream ss;
        ss << file.rdbuf();
        raw = ss.str();
    }
    nlohmann::json json = nlohmann::json::parse(raw);
    //assert_em(json.is_array(), "accounts.json does not contain an array.");
    instances.resize(json.size());
    for (int i = 0; i < json.size(); ++i) {
        instances[i] = std::make_shared<BotInstance>(json[i]["username"], json[i]["password"], json[i]["provider"]);
    }
    return true;
}

std::map<std::vector<std::string>, std::shared_ptr<Pollbot::Command>> commands = {
    {{"test"}, std::make_shared<Pollbot::TestCommand>()},
    {{"test2"}, std::make_shared<Pollbot::Test2Command>()}
};

void Pollbot::Console::handle_console_input(const std::string& line)
{
    if(line.empty()) return;
    std::vector<std::string> args;
    split(line, " ", args);
    if(args.empty()) return;
    for(auto& pair : commands) {
        for(auto& alias : pair.first) {
            if(alias != args[0]) continue;
            args.erase(args.begin());
            pair.second->execute(this, args);
            return;
        }
    }
    ERROR("Command not found \'{}\'", args[0]);
}

void Pollbot::Console::run()
{
    std::string prefix;
    {
        std::stringstream ss;
        ss << BOLDCYAN << "Pollbot " << GREEN << "v" << VERSION << RESET << "$ ";
        prefix = ss.str();
    }
    const char* prefix_ = prefix.c_str();
    while(true) {
        char* line = readline(prefix_);
        handle_console_input(line);
        free(line);
    }
}
