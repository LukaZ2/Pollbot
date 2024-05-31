#include "Firefox.h"
#include <cstdio>
#include <iostream>
#include "../util/util.h"
#include <future>
#include <signal.h>
#include <filesystem>
#include <fstream>
#include <unistd.h>

std::string start_cmd = "ff_no_webdriver/firefox --marionette -no-remote";

void FirefoxInstance::launch(const std::string& args) {
    if(!profile.empty()) {
        std::string profile_dir = std::filesystem::current_path().string() + "/profiles/" + profile;
        std::filesystem::create_directory("profiles");
        std::filesystem::create_directory(profile_dir);
        start_cmd.append(" -profile " + profile_dir);
    }
    if(!args.empty()) start_cmd.append(" " + args);
    start_cmd.append(" > /dev/null 2>&1");
    popen(start_cmd.c_str(), "r");
}

void FirefoxInstance::start_own_pg(const std::string& args) {
    if(pipe(p) < 0) {
        ERROR("Failed to create pipes.");
        exit(-1);
    }

    firefox_process = fork();

    if(firefox_process < 0) {
        ERROR("Failed to fork subprocess.");
        exit(-1);
    }
    if(firefox_process > 0) return;
    setpgid(0, 0);
    reset_signal_handlers();

    launch(args);

    char b;
    read(p[0], &b, 1);
    killpg(0, SIGINT);
    exit(-1);
}

int FirefoxInstance::get_marionette_port() {
    if(profile.empty()) return 2828;
    std::ifstream in("profiles/" + profile + "/MarionetteActivePort");
    std::string port_s;
    {
        std::stringstream ss;
        ss << in.rdbuf();
        port_s = ss.str();
    }
    int port;
    try {
        port = std::stoi(port_s);
    } catch(std::invalid_argument&) {
        ERROR("Invalid marionette port.");
        throw std::runtime_error("");
    }
    return port;
}

void FirefoxInstance::start_own_pg() {
    start_own_pg("");
}

void FirefoxInstance::stop_pg() {
    write(p[1], "", 1);
}

FirefoxInstance::FirefoxInstance(const std::string &profile) : profile(profile) {}

FirefoxInstance::~FirefoxInstance() {
    stop_pg();
}
