#ifndef POLLBOT_LLAMA_H
#define POLLBOT_LLAMA_H
#include <string>
#include <vector>
#include "../util/json.hpp"

namespace LLama {
    class HttpException : public std::exception {
    public:
        int code;
        explicit HttpException(int code) : code(code) {};
    };
    struct Entry {
        std::string prompt;
        std::string response;
    };
    class History {
    public:
        int dynamic_max;
        std::vector<Entry> dynamic_entries;
        std::vector<Entry> static_entries;
        void add_dynamic(Entry entry);
        void add_static(Entry entry);
        nlohmann::json as_json();

        explicit History(int max);
    };
    std::string make_chat_prompt(const std::string& input, History history);
    nlohmann::json make_prompt(const std::string& input, const std::string& system, const nlohmann::json& context = {});
}

#endif //POLLBOT_LLAMA_H
