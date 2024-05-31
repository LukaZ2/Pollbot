#ifndef POLLBOT_LOGGER_H
#define POLLBOT_LOGGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

namespace Logger {
    class Logger {
    public:

        std::vector<spdlog::sink_ptr> sinks;
        std::shared_ptr<spdlog::async_logger> logger;

        Logger();

    };
}

#endif //POLLBOT_LOGGER_H
