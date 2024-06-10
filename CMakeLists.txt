cmake_minimum_required(VERSION 3.26)
project(Pollbot)

set(CMAKE_CXX_STANDARD 17)

#find_package(Curses 6.31232 REQUIRED)
find_package(CURL REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost 1.7 REQUIRED COMPONENTS program_options)

add_executable(Pollbot main.cpp
        program_args.h
        marionette/Firefox.cpp
        marionette/Firefox.h
        util/util.cpp
        util/util.h
        marionette/BaseMarionetteClient.cpp
        marionette/BaseMarionetteClient.h
        util/json.hpp
        marionette/MarionetteClient.cpp
        marionette/MarionetteClient.h
        pollbot/PollPay.cpp
        pollbot/PollPay.h
        pollbot/Provider.h
        util/boilerplate.cpp
        util/boilerplate.h
        llama/llama.cpp
        llama/llama.h
        ui/BotInstance.cpp
        ui/BotInstance.h
        util/Logger.cpp
        util/Logger.h
        pollbot/Provider.cpp
        pollbot/StaticResponse.cpp
        pollbot/StaticResponse.h
        ui/Console.cpp
        ui/Console.h
        pollbot/parser/FormParser.cpp
        pollbot/parser/FormParser.h
        ui/command/ConsoleCommand.cpp
        ui/command/ConsoleCommand.h
        translate/Translate.cpp
        translate/Translate.h
)

include_directories(".")
include_directories(${CURL_INCLUDE_DIR})

target_link_libraries(Pollbot CURL::libcurl)
target_link_libraries(Pollbot spdlog::spdlog)
target_link_libraries(Pollbot readline)
target_link_libraries(Pollbot Boost::program_options)

#target_link_libraries(Pollbot ${CURSES_LIBRARIES})
