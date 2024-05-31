#include "llama.h"
#include <curl/curl.h>
#include "../util/util.h"

LLama::History::History(int max) : dynamic_max(max) {}

void LLama::History::add_dynamic(LLama::Entry entry) {
    if(dynamic_entries.size() >= dynamic_max) dynamic_entries.erase(dynamic_entries.begin());
    dynamic_entries.push_back(entry);
}

void LLama::History::add_static(LLama::Entry entry) {
    static_entries.push_back(entry);
}

nlohmann::json LLama::History::as_json() {
    nlohmann::json json;
    for (auto& entry : static_entries) {
        json.push_back({{"role", "user"}, {"content", entry.prompt}});
        json.push_back({{"role", "assistant"}, {"content", entry.response}});
    }
    for (auto& entry : dynamic_entries) {
        json.push_back({{"role", "user"}, {"content", entry.prompt}});
        json.push_back({{"role", "assistant"}, {"content", entry.response}});
    }
    return json;
}

std::string LLama::make_chat_prompt(const std::string& input, History history) {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
    nlohmann::json post_json;
    post_json["model"] = "surveybot";
    post_json["messages"] = history.as_json();
    post_json["messages"].push_back({{"role", "user"}, {"content", input}});
    post_json["stream"] = false;
    std::string post_data = post_json.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    std::string result;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    int code = curl_easy_perform(curl);
    if(code != CURLE_OK) throw HttpException(code);
    nlohmann::json result_json = nlohmann::json::parse(result);
    return result_json["message"]["content"];
}

nlohmann::json LLama::make_prompt(const std::string& input, const std::string& system, const nlohmann::json& context) {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
    nlohmann::json post_json;
    post_json["model"] = "llama3";
    post_json["prompt"] = input;
    post_json["stream"] = false;
    //if(!context.empty()) post_json["context"] = context;
    post_json["system"] = system;
    post_json["options"]["seed"] = 20;
    post_json["options"]["temperature"] = 0.0;
    // post_json["options"]["num_predict"] = 20;

    std::string post_data = post_json.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    std::string result;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    int code = curl_easy_perform(curl);
    if(code != CURLE_OK) throw HttpException(code);
    nlohmann::json result_json = nlohmann::json::parse(result);
    return result_json;
}
