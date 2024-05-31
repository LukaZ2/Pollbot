#include "StaticResponse.h"
#include <algorithm>
#include <vector>
#include <memory>
#include <util/util.h>
#include "parser/FormParser.h"
#include <llama/llama.h>
#include <limits.h>
#include <translate/Translate.h>

// translate(text(), \"ABCDEFGHIJKLMNOPQRSTUVWXYZ\", \"abcdefghijklmnopqrstuvwxyz\")

nlohmann::json text_responses;
nlohmann::json multiple_choice_responses;

bool load_file(const std::string& filename, nlohmann::json& to) {
    try {
        if(!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename)) return false;
        std::ifstream file(filename);
        std::stringstream ss;
        ss << file.rdbuf();
        to = nlohmann::json::parse(ss.str());
    } catch(std::exception& e) {
        ERROR(e.what());
        return false;
    }
    return true;
}

bool StaticResponse::load_static()
{
    assert_em(load_file("static_response/textinput.json", text_responses), "Failed to load textinput.json.");
    assert_em(load_file("static_response/multiplechoice.json", multiple_choice_responses), "Failed to load multiplechoice.json.");
    return true;
}

bool match_criteria(const std::string& input, const std::vector<std::string>& options, nlohmann::json& criteria) {
    std::string input_lower = input;
    std::transform(input.begin(), input.end(), input_lower.begin(), tolower);
    std::remove_if(input_lower.begin(), input_lower.end(), [](auto c)->bool {
        return isdigit(c) || ispunct(c);
    });
    for(int i = 0; i < criteria.size(); i++) {
        auto& entry = criteria[i];
        bool require_all = entry.contains("require_all") ? (bool)entry["require_all"] : false;
        bool isolated = entry.contains("isolated") ? (bool)entry["isolated"] : false;
        if(entry.contains("contains")) {

            int found_n = 0;
            for(auto& text : entry["contains"]) {
                int pos = input_lower.find(text);
                bool found = pos != std::string::npos;
                if(found && isolated) {
                    std::string str(text);
                    if(pos+str.length() < input_lower.length() && isalpha(input_lower.at(pos+str.length()+1))) found = false;
                    if(pos > 0 && isalpha(input_lower.find(pos-1))) found = false;
                }
                if(found) found_n++;
                if(found && !require_all) return true;
                if(!found && require_all) break;
            }
            if(found_n != 0 && found_n >= entry["contains"].size()) return true;

            continue;

        }
        if(entry.contains("matches")) {
            if(input_lower == entry["matches"]) return true;
            continue;
        }
        if(entry.contains("options")) {

            int found_n = 0;
            for(auto& option : options) {
                bool found = match_criteria(option, {}, entry["options"]);
                if(found) found_n++;
                if(found && !require_all) return true;
            }

            if(found_n != 0 && found_n >= entry["options"].size()) return true;

            continue;
        }
    }
    return false;
}

bool get_static_text_response(const std::string& title, std::string& out) {
    for(int i = 0; i < text_responses.size(); i++) {
        if(!match_criteria(title, {}, text_responses[i]["search"])) continue;
        out = text_responses[i]["response"];
        return true;
    }
    return false;
}

bool get_static_multiple_choice_response(const std::string& title, const std::vector<std::string>& options, int& out) {
    for(int i = 0; i < multiple_choice_responses.size(); i++) {
        if(!match_criteria(title, options, multiple_choice_responses[i]["search"])) continue;
        for(int j = 0; j < options.size(); j++) {
            if(!match_criteria(options[j], {}, multiple_choice_responses[i]["response"])) continue;
            out = j;
            return true;
        }
    }
    return false;
}


void StaticResponse::get_text_response(const std::string& title, std::string& out, nlohmann::json& context, const std::string& prompt_custom)
{
    if(get_static_text_response(title, out)) return;
    std::string prompt = title;

    // int confidence;
    // std::string detect_language = Translate::detect_language(prompt, confidence);
    // if(detect_language != "en" && confidence > 50) Translate::translate(prompt, detect_language, "en");

    DEBUG("Text-title: {}", title);

    prompt += prompt_custom.empty() ? "\n\nKeep your answer as short as possible." : prompt_custom;
    nlohmann::json response = LLama::make_prompt(prompt, "You are a person trying to qualify for an online survey. Give answers that the survey is likely looking for and keep them short.", context);
    out = response["response"];
    rm_umlaut(out);
    context = response["context"];
    DEBUG("Response: {}", out);
}

void StaticResponse::get_number_response(const std::string& title, std::string& out, int min, int max, int step, nlohmann::json& context)
{
    DEBUG("Number-title: {}", title);

    std::string prompt_custom = "Answer using a number";

    if(min != INT_MIN && max != INT_MAX) {
        prompt_custom += " between ";
        prompt_custom += std::to_string(min);
        prompt_custom += " and ";
        prompt_custom += std::to_string(max);
    }
    else if(min != INT_MIN) {
        prompt_custom += " above ";
        prompt_custom += std::to_string(min);
    }
    else if(max != INT_MAX) {
        prompt_custom += " below ";
        prompt_custom += std::to_string(max);
    }

    if(!title.empty()) get_text_response(title, out, context, prompt_custom);

    std::vector<int> response;
    if(!get_numbers(out, response) || response[0] < min || response[0] > max) response.push_back(((max-min)/2)+min);
    out = std::to_string(((int)(response[0]/step))*step);
}

void try_translate_mc(std::string& title, std::vector<std::string>& options) {
    std::string blob = title;
    for(int i = 0; i < options.size(); i++) {
        blob+="\n";
        blob+=options[i];
    }
    int confidence;
    std::string detected = Translate::detect_language(blob, confidence);
    if(detected == "en" || confidence < 50) return;
    Translate::translate(blob, detected, "en");

    std::vector<std::string> blob_split;
    split(blob, "\n", blob_split);

    if(blob_split.size()-1 != options.size()) {
        ERROR("Failed to translate. blob={}", blob);
        return;
    }

    title = blob_split[0];
    for(int i = 0; i < options.size(); i++) {
        options[i] = blob_split[i+1];
    }
}

bool extract_json_answer(const std::string& str, std::vector<int>& out) {
    size_t first = str.find("{\"");
    if(first == std::string::npos) {
        char c = get_first_alpha(str);
        assert_(c != '\0');
        size_t num = get_alphabet().find(c);
        return !out.empty();
    }
    std::string to_split = str.substr(first+2, str.length());
    std::vector<std::string> splitted;
    split(to_split, "{\"", splitted);

    for(auto& s : splitted) {
        size_t num = get_alphabet().find(s.at(0));
        if(num != std::string::npos) out.push_back(num);
    }
    return !out.empty();
}

void StaticResponse::get_multiple_choice_response(const std::string& title, const std::vector<std::string>& options, std::vector<int>& out, nlohmann::json& context)
{
    int static_response = -1;
    if(get_static_multiple_choice_response(title, options, static_response)) {
        out = {static_response};
        return;
    }

    if(options.size() > 15) {
        out = {2, 5, 10};
        return;
    }

    std::string title_translated = title;
    std::vector<std::string> options_translated = options;
    try_translate_mc(title_translated, options_translated);

    std::string prompt = title_translated;

    // prompt = "7+18=?";
    // options_translated = {
    //     "14",
    //     "3",
    //     "12",
    //     "25",
    //     "17",
    // };

    prompt += "\n\nProvided options:";
    for(int i = 0; (i < options_translated.size()) && (i < 26); i++) {
        prompt += "\n{\"";
        prompt += get_alphabet().at(i);
        prompt += "\": \"";
        prompt += options_translated[i];
        prompt += "\"}";
    }

    DEBUG("Prompt:\n{}", prompt);

    nlohmann::json response_json = LLama::make_prompt(prompt, "You are a person taking an online survey. Choose the options that are most realistic.", context);
    std::string response = response_json["response"];
    context = response_json["context"];


    DEBUG("Response: {}", response);

    // for(int i = 0; i < options_translated.size(); i++) {
    //     if(response != options_translated[i]) continue;
    //     out.push_back(i);
    //     return;
    // }

    if(!extract_json_answer(response, out)) out.push_back(0);

    auto it = out.begin();
    while(it != out.end()) {
        // (*it)--;
        if((*it) >= options_translated.size() || (*it) < 0) it = out.erase(it);
        else it++;
    }
    if(out.empty()) out = {0};
}
