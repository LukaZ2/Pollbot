#include "BotInstance.h"
#include <utility>
#include <thread>
#include <util/util.h>
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include <pollbot/parser/FormParser.h>
#include <util/boilerplate.h>
#include <program_args.h>

struct blacklisted_site {
    enum type {
        EQUALS,
        CONTAINS,
        START
    } t;
    std::string text;
    bool ignore_case;
};
NLOHMANN_JSON_SERIALIZE_ENUM(blacklisted_site::type, {
{blacklisted_site::EQUALS, "EQUALS"},
{blacklisted_site::CONTAINS, "CONTAINS"},
{blacklisted_site::START, "START"}
});
void from_json(const nlohmann::json& json, blacklisted_site& blacklisted_site) {
    json["text"].get_to(blacklisted_site.text);
    json["type"].get_to(blacklisted_site.t);
    blacklisted_site.ignore_case = json.contains("ignore_case") ? (bool)json["ignore_case"] : false;
}
std::vector<blacklisted_site> blacklisted_sites;

#define SITE_BLACKLIST_FILE "static_response/site_blacklist.json"
void load_site_blacklist()
{
    std::ifstream file(SITE_BLACKLIST_FILE);
    if(!file.is_open()) {
        ERROR("Failed to open {}.", SITE_BLACKLIST_FILE);
        return;
    }
    std::stringstream raw;
    raw << file.rdbuf();
    blacklisted_sites = nlohmann::json::parse(raw.str());
}

bool is_blacklisted_site(std::string url)
{
    for(auto& site : blacklisted_sites) {
        if(site.ignore_case) std::transform(url.begin(), url.end(), url.begin(), tolower);
        switch(site.t) {
            case blacklisted_site::EQUALS: if(site.text == url) return true;
                break;

            case blacklisted_site::CONTAINS: if(url.find(site.text) != std::string::npos) return true;
                break;

            case blacklisted_site::START: if(url.length() >= site.text.length() && url.substr(0, site.text.length()) == site.text) return true;
                break;

            default:
                break;
        }
    }
    return false;
}


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

        INFO("Instance is now {}ACTIVE{}.", GREEN, RESET);

        while(true) {
            assert_em(close_all(client.get()), "close_all() failed.");
            int tries = 0;
            while(!provider->enter(username, password)) {
                tries++;
                assert_em(tries < 3, "Provider->enter() failed too many times.");
            }
            sleep_ms(1000);
            if(!provider->start_poll()) {
                ERROR("Provider->start_poll() failed.");
                continue;
            }

            tablist tabs = get_tabs(client.get());
            {
                tablist newtabs;
                tablist tmp_tabs;
                tries = 0;
                while(!element_exists(client.get(), CSS_SEL("form")) && (newtabs = get_new_tabs((tmp_tabs = get_tabs(client.get())), tabs)).empty()) {
                    sleep_ms(1000);
                    tries++;
                    assert_em(tries < 18, "Survey start timed out.");
                    continue;
                }
                tabs = tmp_tabs;
                if(!newtabs.empty() && !client->switch_to_window(newtabs[0]).get().success) ERROR("Failed to switch to window {}.", newtabs[0]);
                sleep_ms(2000);
            }

            Parser::FormCache question_cache;

            while(true) {

                DEBUG("Parsing page...");

                Parser::FormParser* parser = Parser::get_parser(get_url(client.get()));

                tries = 0;
                nlohmann::json tree = nullptr;
                nlohmann::json tmp_tree;
                tablist tmp_tabs;
                while(true) {
                    tablist newtabs;
                    if(!(newtabs = get_new_tabs((tmp_tabs = get_tabs(client.get())), tabs)).empty()) {
                        assert_em(client->switch_to_window(newtabs[0]).get().success, "Failed to switch to new window.");
                        tabs = tmp_tabs;
                        sleep_ms(100);
                    }

                    if(provider->on_main_page(client.get())) goto endloop;
                    if(std::string url = get_url(client.get()); is_blacklisted_site(url)) {
                        INFO("Exiting blacklisted page {}{}{}.", YELLOW, url, RESET);
                        goto endloop;
                    }
                    assert_em(parser->get_form_tree(client.get(), tmp_tree), "get_form_tree() failed.");
                    if(tree == tmp_tree) break;
                    sleep_ms(4000);
                    if(!parser->has_interactables(tmp_tree)) continue;
                    tree = tmp_tree;
                }

                for(int i = 0; i < tree.size(); i++) {
                    DEBUG(tree[i].dump());
                }

                if(!parser->handle_form_tree(client.get(), tree, question_cache)) {
                    ERROR("handle_form_tree() failed.");
                    break;
                }

                sleep_ms(2000);
            }
            endloop: continue;
        }

    } catch(std::exception& e) {
        WARN("Instance exited: {}", e.what());
    }
    return true;
}

void BotInstance::worker() {
    firefoxInstance = nullptr;
    firefoxInstance = std::make_shared<FirefoxInstance>(username);
    firefoxInstance->start_own_pg();

    main_cycle();
    if(debug_mode) {
        DEBUG("Instance is now {}PAUSED{}.", YELLOW, RESET);
        stop_;
    }
    INFO("Stopping account instance {}...", username);
    stop();
    client->join();
    client = nullptr;
    state = INACTIVE;
    INFO("Instance is now {}INACTIVE{}.", RED, RESET);
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
    INFO("Starting account instance {} on provider {}...", username, provider_name);
    task = std::async(std::launch::async, &BotInstance::worker, this);
    return true;
}

void BotInstance::stop() {
    if(state == INACTIVE || state == STOPPING) return;
    state = STOPPING;
    firefoxInstance = nullptr;
}
