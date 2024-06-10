#include "llama.h"
#include <curl/curl.h>
#include "../util/util.h"

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
    if(code != CURLE_OK) throw std::runtime_error(std::string("LLama HTTP request failed with code ") + std::to_string(code) + ".");
    nlohmann::json result_json = nlohmann::json::parse(result);
    return result_json;
}
