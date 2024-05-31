#include "BotInstance.h"
#include <utility>
#include <thread>
#include "../util/util.h"
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include "../pollbot/parser/FormParser.h"
#include "../util/boilerplate.h"

BotInstance::BotInstance(std::string username, std::string password, std::string provider_name) :
state(INACTIVE), username(std::move(username)),password(std::move(password)), provider_name(std::move(provider_name)) {}

BotInstance::~BotInstance() {
    stop();
}

bool BotInstance::main_cycle() {
    try {

        Timer timer(8000);
        bool success = false;
        while (!timer.passed()) {
            sleep_ms(2000);
            if (!client->start(firefoxInstance->get_marionette_port())) continue;
            success = true;
            break;
        }
        assert_em(success, "Connection to marionette timed out.");
        assert_em(client->new_session().get().success, "New session failed.");

        state = ACTIVE;

        stop_;

        while(true) {
            int tries = 0;
            while(!provider->enter(username, password)) {
                tries++;
                assert_em(tries < MAX_TRIES, "Provider->enter() failed too many times.");
            }
            sleep_ms(1000);
            if(!provider->start_poll()) {
                ERROR("Provider->start_poll() failed.");
                continue;
            }

            Parser::FormCache question_cache;

            sleep_ms(2000);

            if(!wait_until_page_stable(client.get(), 3)) {
                ERROR("Page not stable.");
                break;
            }

            while(true) {

                Parser::FormParser* parser = Parser::get_parser(get_url(client.get()));

                nlohmann::json tree;
                if(!parser->get_form_tree(client.get(), tree)) {
                    ERROR("get_form_tree() failed.");
                    break;
                }

                for(int i = 0; i < tree.size(); i++) {
                    DEBUG(tree[i].dump());
                }

                tablist tabs = get_tabs(client.get());

                if(!parser->handle_form_tree(client.get(), tree, question_cache)) {
                    ERROR("handle_form_tree() failed.");
                    break;
                }

                sleep_ms(2000);

                if(!wait_until_page_stable(client.get(), 3)) {
                    ERROR("Page not stable.");
                    break;
                }

                tablist newtabs = get_new_tabs(client.get(), tabs);
                if(newtabs.size() > 0) client->switch_to_window(newtabs[0]);
                sleep_ms(200);

                if(provider->on_main_page(client.get())) break;
            }
            DEBUG("Done");
            client->join();
        }

    } catch(std::exception& e) {
        INFO(e.what());
        INFO("Exception caught");
    }
    return true;
}

void BotInstance::worker() {
    firefoxInstance = nullptr;
    firefoxInstance = std::make_shared<FirefoxInstance>(username);
    firefoxInstance->start_own_pg();

    main_cycle();
    stop();
    client->join();
    client = nullptr;
    state = INACTIVE;
}

bool BotInstance::launch() {
    if(state != INACTIVE) return false;
    if(client != nullptr && client->is_running()) return false;
    client = nullptr;
    client = std::make_shared<MarionetteClient>();

    provider = nullptr;
    provider = create_provider(client.get(), provider_name);
    assert_(provider != nullptr);

    state = STARTING;
    task = std::async(std::launch::async, &BotInstance::worker, this);
    return true;
}

void BotInstance::stop() {
    if(state == INACTIVE || state == STOPPING) return;
    state = STOPPING;
    firefoxInstance = nullptr;
}
