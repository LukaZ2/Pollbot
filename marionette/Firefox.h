#ifndef POLLBOT_FIREFOX_H
#define POLLBOT_FIREFOX_H
#include <unistd.h>
#include <thread>
#include <string>
#include <memory>

class FirefoxInstance  {
public:
    typedef std::shared_ptr<FirefoxInstance> ptr;

    std::string profile;
    pid_t firefox_process;
    int p[2];
    void start_own_pg(const std::string& args);
    void start_own_pg();
    void stop_pg();
    int get_marionette_port();
    void launch(const std::string& args);

    explicit FirefoxInstance(const std::string &profile);
    ~FirefoxInstance();
};

#endif //POLLBOT_FIREFOX_H
