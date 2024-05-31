#include "BaseMarionetteClient.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <memory>
#include "../util/util.h"

nlohmann::json MarionettePacket::as_json_array() {
    return nlohmann::json::array({0, messageId, command, body});
}

std::string MarionettePacket::Response::Error::get() {
    return error + ", " + message + ", " + stacktrace;
}

MarionettePacket::MarionettePacket(int messageId, const std::string &command, const nlohmann::json &body) : messageId(
        messageId), command(command), body(body) {
}

bool BaseMarionetteClient::read_packet(nlohmann::json & json) {
    std::string length_s;
    char b = '\0';
    while(b != ':') {
        if(read(fd, &b, 1) <= 0) return false;
        length_s += b;
    }
    int length;
    try {
        length = std::stoi(length_s);
    } catch(std::invalid_argument&) {
        ERROR("std::stoi failed. length_s={}", length_s);
        return false;
    }
    int remaining = length;
    char packet[length+1];
    int c_read;
    while((c_read = read(fd, packet+(length-remaining), remaining)) != -1) {
        remaining -= c_read;
        if(remaining == 0) break;
    }
    if(c_read == -1) return false;
    packet[length] = '\0';
    json = nlohmann::json::parse(&packet[0]);
    return true;
}

void BaseMarionetteClient::listen() {
    nlohmann::json json;
    while (read_packet(json)) {
        MarionettePacket::Response packet = {
                json[2].is_null(),
                MarionettePacket::Response::Error(),
                json[3]
                };
        int index = json[1];
        index--;
        if(!packet.success) {
            packet.error = {json[2]["error"], json[2]["message"], json[2]["stacktrace"]};
#ifndef NDEBUG
            WARN("---------- REQUEST -----------");
            WARN(sent_packets[index]->as_json_array().dump());
            WARN("---------- RESPONSE ----------");
            WARN(packet.error.error);
            WARN(packet.error.message);
            WARN("------------------------------");
#endif
        }
        sent_packets[index]->response.set_value(packet);
        std::lock_guard l(sent_packets_mtx);
        sent_packets[index] = nullptr;
    }
    std::lock_guard l(sent_packets_mtx);
    for (int i = 0; i < PACKETS_MAX; ++i) {
        if(sent_packets[i] == nullptr) continue;
        sent_packets[i]->response.set_value({false, {"Marionette socket closed.", "", ""}, {}});
    }
    close(fd);
}

bool BaseMarionetteClient::start(int port) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        ERROR("Failed to create socket file descriptor.");
        return false;
    }
    sockaddr_in localhost;
    localhost.sin_family = AF_INET;
    localhost.sin_port = htons(port);
    if(inet_pton(AF_INET, "127.0.0.1", &localhost.sin_addr) <= 0) {
        ERROR("Failed to resolve localhost.");
        return false;
    }
    if(connect(fd, (sockaddr*)&localhost, sizeof(localhost)) < 0) {
        ERROR("Failed to connect to Marionette.");
        return false;
    }
    nlohmann::json json;
    if(!read_packet(json)) return false;
    task = std::async(std::launch::async, &BaseMarionetteClient::listen, this);
    return true;
}

bool BaseMarionetteClient::start() {
    return start(2828);
}

void BaseMarionetteClient::stop() {
    close(fd);
}

std::future<MarionettePacket::Response> BaseMarionetteClient::queue_packet(std::string command, nlohmann::json body) {
    while(true) {
        std::lock_guard l(sent_packets_mtx);
        for (int i = 0; i < PACKETS_MAX; ++i) {
            if (sent_packets[i] != nullptr) continue;
            sent_packets[i] =  std::make_shared<MarionettePacket>(i+1, command, body);
            if(!send_packet(sent_packets[i])) throw std::runtime_error("Marionette socket closed.");
            return sent_packets[i]->response.get_future();
        }
    }
}

bool BaseMarionetteClient::send_packet(MarionettePacket::ptr packet) {
    std::string data = to_string(packet->as_json_array());
    int length = data.length();
    //char raw_packet[2+((int)log(length))+length];
    std::string length_s = std::to_string(length);
    length_s += ":";
    return (write(fd, length_s.c_str(), length_s.length()) != -1) && (write(fd, data.c_str(), length) != -1);
}

BaseMarionetteClient::~BaseMarionetteClient() {
    if(is_running())
    close(fd);
}
using namespace std::chrono_literals;
bool BaseMarionetteClient::is_running() {
    try {
        return task.wait_for(0ms) != std::future_status::ready;
    } catch(std::future_error&) {}
    return false;
}

void BaseMarionetteClient::join() {
    if(is_running())
    task.get();
}
