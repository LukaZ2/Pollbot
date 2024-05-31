#ifndef POLLBOT_POLLPAY_H
#define POLLBOT_POLLPAY_H
#include "Provider.h"

class PollPay : public Provider {
public:
    double balance;

    explicit PollPay(MarionetteClient *client);

    std::string url() override;
    bool login(std::string username, std::string password) override;
    double get_balance() override;
    bool start_poll() override;

    bool enter(std::string username, std::string password) override;

    bool on_main_page(MarionetteClient* client) override;

private:
    bool login_google(std::string& username, std::string& password);
};

#endif //POLLBOT_POLLPAY_H
