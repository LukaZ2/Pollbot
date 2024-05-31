#ifndef STATICRESPONSE_H
#define STATICRESPONSE_H

#include <string>
#include <marionette/MarionetteClient.h>
#include <vector>

namespace StaticResponse {

    bool load_static();

    void get_text_response(const std::string& title, std::string& out, nlohmann::json& context, const std::string& prompt_custom = "");
    void get_number_response(const std::string& title, std::string& out, int min, int max, int step, nlohmann::json& context);
    void get_multiple_choice_response(const std::string& title, const std::vector<std::string>& options, std::vector<int>& out, nlohmann::json& context);
}

#endif // STATICRESPONSE_H
