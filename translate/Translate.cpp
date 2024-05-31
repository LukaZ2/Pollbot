#include "Translate.h"
#include <curl/curl.h>
#include <util/util.h>
#include <util/json.hpp>

std::string Translate::detect_language(const std::string& str, int& confidence)
{
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/detect");
    curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string post = nlohmann::json({{"q", str}}).dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

    std::string result;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    int code = curl_easy_perform(curl);

    if(code != CURLE_OK) {
        ERROR("detect_language() failed with code {}. post={}, result={}", code, post, result);
        return {};
    }
    nlohmann::json result_json = nlohmann::json::parse(result);
    if(!result_json.is_array() || result_json.empty()) {
        ERROR("detect_language() failed with result {}.", result);
        return {};
    }
    if(result_json.size() == 1) {
        confidence = result_json[0]["confidence"];
        return result_json[0]["language"];
    }

    nlohmann::json& best = result_json[0];

    for(int i = 1; i < result_json.size(); i++) {
        if((int)result_json[i]["confidence"] <= (int)best["confidence"]) continue;
        best = result_json[i];
    }
    confidence = best["confidence"];
    return best["language"];
}

void Translate::translate(std::string& str, const std::string& lang_source, const std::string& lang_target)
{
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/translate");
    curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string post = nlohmann::json({
        {"q", str},
        {"source", lang_source},
        {"target", lang_target},
    }).dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

    std::string result;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    int code = curl_easy_perform(curl);

    if(code != CURLE_OK) {
        ERROR("translate() failed with code {}. post={}, result={}", code, post, result);
        return;
    }
    nlohmann::json result_json = nlohmann::json::parse(result);

    if(!result_json.contains("translatedText")) {
        ERROR("translate() failed with result {}", result);
        return;
    }
    str = result_json["translatedText"];
}
