#include "ConsoleCommand.h"
#include <ui/Console.h>
#include <util/boilerplate.h>
#include <pollbot/parser/FormParser.h>
#include <util/util.h>
#include <pollbot/StaticResponse.h>

Parser::FormCache cache;

void Pollbot::TestCommand::execute(Console* console, std::vector<std::string>& args)
{
    auto& instance = console->instances[0];

    instance->client->switch_to_window(get_tabs(instance->client.get())[std::stoi(args[0])]);

    nlohmann::json tree;
    Parser::FormParser* parser = Parser::get_parser(get_url(instance->client.get()));
    parser->get_form_tree(instance->client.get(), tree);

    for(int i = 0; i < tree.size(); i++) {
        DEBUG(tree[i].dump());
    }

    parser->handle_form_tree(instance->client.get(), tree, cache);

}

void Pollbot::Test2Command::execute(Console* console, std::vector<std::string>& args)
{
    if(args.size() < 3) return;

    std::vector<int> out;
    std::vector<std::string> options = args;
    options.erase(options.begin());
    nlohmann::json ctx;
    StaticResponse::get_multiple_choice_response(args[0], options, out, ctx);
    for(auto& a : out) {
        INFO("Out: {}", a);
    }
}
