#ifndef POLLBOT_BOTINSTANCE_H
#define POLLBOT_BOTINSTANCE_H
#include <memory>
#include <marionette/MarionetteClient.h>
#include <marionette/Firefox.h>
#include <pollbot/Provider.h>

void load_site_blacklist();
bool is_blacklisted_site(std::string url);

class BotInstance {
    void worker();
    bool main_cycle();
public:
    typedef std::shared_ptr<BotInstance> ptr;

    enum State {
        ACTIVE,
        INACTIVE,
        STARTING,
        STOPPING,
        PAUSED,
    };

    MarionetteClient::ptr client;
    FirefoxInstance::ptr firefoxInstance;
    Provider::ptr provider;

    std::future<void> task;
    std::atomic<State> state;
    std::string username;
    std::string password;
    std::string provider_name;

    BotInstance(std::string username, std::string password, std::string  provider_name);
    ~BotInstance();
    bool launch();
    void stop();
};

#endif //POLLBOT_BOTINSTANCE_H
