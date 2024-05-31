#include "Provider.h"
#include <utility>
#include "PollPay.h"
#include <algorithm>

struct Pair {
    std::string name;
    Provider::ptr (*create)(MarionetteClient*);
    Pair(std::string name, Provider::ptr (*create)(MarionetteClient*)) : name(std::move(name)), create(create) {}
};

template<class T>
Provider::ptr provider_factory(MarionetteClient* client) {
    return std::make_shared<T>(client);
}
#define PV(name, type) {name, provider_factory<type>}
std::vector<Pair> providers = {
            PV("POLLPAY", PollPay)
    };

Provider::ptr create_provider(MarionetteClient* client, const std::string &name) {
    for (auto& provider : providers) {
        if(provider.name == name) return provider.create(client);
    }
    return nullptr;
}
