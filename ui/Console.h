#ifndef POLLBOT_MAINMENU_H
#define POLLBOT_MAINMENU_H
#include "BotInstance.h"
#include <map>

namespace Pollbot {

    class Console {
    public:

        std::vector<BotInstance::ptr> instances;
        std::stringstream ostream;

        bool load_accounts();
        void run();
        void handle_console_input(const std::string& line);
    };
}

#endif //POLLBOT_MAINMENU_H
