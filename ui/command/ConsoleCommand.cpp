#include "ConsoleCommand.h"
#include <ui/Console.h>
#include <util/boilerplate.h>
#include <pollbot/parser/FormParser.h>
#include <util/util.h>
#include <pollbot/StaticResponse.h>

Parser::FormCache cache;

void Pollbot::StartCommand::execute(Console* console, std::vector<std::string>& args)
{
    if(args.empty()) {
        ERROR("Usage: start [index]");
        return;
    }
    int index;
    try {
        index = std::stoi(args[0]);
    } catch(std::invalid_argument&) {
        ERROR("Invalid index.");
        return;
    }
    if(index >= console->instances.size() || index < 0) {
        ERROR("Index out of bounds");
        return;
    }
    console->instances[index]->launch();
}


void Pollbot::TestCommand::execute(Console* console, std::vector<std::string>& args)
{
    auto& client = console->instances[0]->client;

    client->switch_to_window(get_tabs(client.get())[std::stoi(args[0])]);
    if(is_blacklisted_site(get_url(client.get()))) {
        ERROR("Site is blacklisted.");
        return;
    }

    nlohmann::json tree;
    Parser::FormParser* parser = Parser::get_parser(get_url(client.get()));
    parser->get_form_tree(client.get(), tree);

    for(int i = 0; i < tree.size(); i++) {
        DEBUG(tree[i].dump());
    }

    parser->handle_form_tree(client.get(), tree, cache);

}

void Pollbot::Test2Command::execute(Console* console, std::vector<std::string>& args)
{
    if(args.empty()) return;
    DEBUG(is_blacklisted_site(args[0]));
    return;
}
