#ifndef POLLBOT_LLAMA_H
#define POLLBOT_LLAMA_H
#include <string>
#include <vector>
#include "../util/json.hpp"

namespace LLama {
    nlohmann::json make_prompt(const std::string& input, const std::string& system, const nlohmann::json& context = {});
}

#endif //POLLBOT_LLAMA_H
