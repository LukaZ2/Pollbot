#ifndef POLLBOT_BASEMARIONETTECLIENT_H
#define POLLBOT_BASEMARIONETTECLIENT_H
#include <thread>
#include <vector>
#include "../util/json.hpp"
#include <string>
#include <future>
#include <mutex>

struct MarionettePacket {
    typedef std::shared_ptr<MarionettePacket> ptr;

    struct Response {
        struct Error {
            std::string error;
            std::string message;
            std::string stacktrace;
            std::string get();
        };
        bool success;
        Error error;
        nlohmann::json body;
    };

    int messageId;
    std::string command;
    nlohmann::json body;

    std::promise<Response> response;

    nlohmann::json as_json_array();

    MarionettePacket(int messageId, const std::string &command, const nlohmann::json &body);
};

class BaseMarionetteClient {
public:
    std::mutex sent_packets_mtx;
    bool start(int port);
    bool start();
    void stop();
    std::future<MarionettePacket::Response> queue_packet(std::string command, nlohmann::json body);
    ~BaseMarionetteClient();
    bool is_running();
    void join();

private:
#define PACKETS_MAX 10
    int fd;
    std::future<void> task;
    MarionettePacket::ptr sent_packets[PACKETS_MAX] = {};
    void listen();
    bool read_packet(nlohmann::json&);
    bool send_packet(MarionettePacket::ptr packet);
};

#endif //POLLBOT_BASEMARIONETTECLIENT_H