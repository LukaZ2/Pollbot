#ifndef CONSOLECOMMAND_H
#define CONSOLECOMMAND_H

#include <string>
#include <vector>

namespace Pollbot {

    class Console;

    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute(Console* console, std::vector<std::string>& args) = 0;
    };

    class StartCommand : public Command {
    public:
        void execute(Console * console, std::vector<std::string> & args) override;
    };

    class TestCommand : public Command {
    public:
        void execute(Console* console, std::vector<std::string> & args) override;
    };

    class Test2Command : public Command {
    public:
        void execute(Console * console, std::vector<std::string> & args) override;
    };
}

#endif // CONSOLECOMMAND_H
