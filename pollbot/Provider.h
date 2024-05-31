#ifndef POLLBOT_PROVIDER_H
#define POLLBOT_PROVIDER_H
#include <string>
#include <marionette/MarionetteClient.h>

class Provider {
public:
    typedef std::shared_ptr<Provider> ptr;
    MarionetteClient* client;
    explicit Provider(MarionetteClient *client) : client(client) {}
    virtual ~Provider() = default;
    virtual std::string url() { return ""; }
    virtual bool enter(std::string username, std::string password) {}
    virtual bool login(std::string username, std::string password) {}
    virtual double get_balance() { return -1; }
    virtual bool start_poll() { return false; }
    virtual bool on_main_page(MarionetteClient* client) {return false;}
};

Provider::ptr create_provider(MarionetteClient* client, const std::string& name);

#endif //POLLBOT_PROVIDER_H
