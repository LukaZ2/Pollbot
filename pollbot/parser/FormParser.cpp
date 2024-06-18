#include "FormParser.h"
#include <util/boilerplate.h>
#include <util/util.h>
#include <map>
#include <pollbot/StaticResponse.h>
#include <limits.h>

Parser::FormParser common;
std::map<std::string, Parser::FormParser::ptr> parsers = {
//{"https://spectrumsurveys.com", std::make_shared<Parser::SpectrumSurveyParser>()}
};

Parser::FormParser* Parser::get_parser(const std::string& url)
{
    for(auto& parser : parsers) {
        if(url.find(parser.first) != std::string::npos) return parser.second.get();
    }
    return &common;
}


bool Parser::FormParser::get_form_tree(MarionetteClient* client, nlohmann::json& tree)
{
    mResponse util_response = execute_js_file(client, "js/util.js");
    if(!util_response.success) {
        CRITICAL("Marionette could not execute util.js. Error: {}", util_response.error.get());
        return false;
    }
    mResponse response = execute_js_file(client, "js/get_form_tree.js");
    if(!response.success) {
        CRITICAL("Marionette could not execute get_form_tree.js. Error: {}", response.error.get());
        return false;
    }
    tree = response.body["value"];
    return true;
}

bool Parser::FormParser::handle_captchas(MarionetteClient* client, std::future<void>& done)
{
    done = std::async(std::launch::async, [client] {

    });
    return true;
}


bool Parser::FormParser::has_interactables(const nlohmann::json& tree)
{
    for(int i = 0; i < tree.size(); i++) {
        if(tree[i]["type"] == "txt") continue;
        return true;
    }
    return false;
}


bool find_title_js(MarionetteClient* client, const std::string& node, std::string& out)
{
    mResponse rect = client->get_element_rect(node).get();
    assert_(rect.success);

    int x = rect.body["x"];
    int y = rect.body["y"];

    nlohmann::json args = nlohmann::json::array();
    args.push_back({{"x", x}, {"y", y}});

    mResponse title = execute_js_file(client, "js/find_title.js", args);
    assert_em(title.success, "find_title.js failed.");

    try {
        assert_em(!title.body["value"].empty() && !title.body.empty(), "find_title.js result empty.");
        out = title.body["value"][0];
    } catch(nlohmann::json::exception& e) {
        ERROR("find_title.js result error. Error {}", e.what());
        return false;
    }

    return true;
}

template<class T>
int count_tree(nlohmann::json& tree, const std::string& key, const T& value) {
    int c = 0;
    for(int i = 0; i < tree.size(); i++) {
        if(tree[i][key] == value) c++;
    }
    return c;
}

int count_tree_contains(nlohmann::json& tree, const std::string& key) {
    int c = 0;
    for(int i = 0; i < tree.size(); i++) {
        if(tree[i].contains(key)) c++;
    }
    return c;
}

bool get_prev_text_all(nlohmann::json& tree, int i, std::string& out, bool include_first = true) {
    if(include_first) out = tree[i]["text"];
    if(i == 0) return !out.empty();

    int ly = tree[i].contains("rects") ? (int)tree[i]["rects"]["y"] : 0;

    for(int j = i-1; j >= 0; j--) {
        auto& type = tree[j]["type"];
        if(type != "txt") {
            if(tree[j].contains("lcp")) return !out.empty();
            if(!out.empty()) return true;
        }
        int y = tree[i].contains("rects") ? (int)tree[i]["rects"]["y"] : 0;
        if(ly > 0 && y > 0 && (ly-y) > 100) return !out.empty();
        out.insert(0, " ");
        out.insert(0, tree[j]["text"]);
    }

    return !out.empty();
}

bool get_text(nlohmann::json& tree, int i, std::string& out) {
    if(get_prev_text_all(tree, i, out)) return true;
    if(tree[i].contains("text")) {
        out = tree[i]["text"];
        return !out.empty();
    }
    if((i > 0) && tree[i-1]["type"] == "txt") {
        out = tree[i-1]["text"];
        return !out.empty();
    }
    return false;
}

struct reject_text {
    std::string text;
    bool require_equals;
    bool check_options = false;
};
std::vector<reject_text> reject_text = {
    {"nein", true},
    {"no", true},
    {"datenschutzbedingungen", true, true},
    {"datenschutzrichtlinien", true, true},
    {"datenschutzrichtlinie", true, true},
    {"datenschutzerklärung", true, true},
    {"nutzungsbedingungen", true, true},
    {"datenschutzhinweis", true, true},
    {"datenschutz-bestimmungen", true, true},
    {"geschäftsbedingungen", false},
    {"allgemeine geschäftsbedingungen", true, true},
    {"cookie consent", false},
    {"terms and agreements", true, true},
    {"privacy policy", true, true},
    {"privacy-policy", true, true},
    {"back", false},
    {"previous", false},
    {"zurück", false},
    {"reject", false},
    {"ablehnen", false},
    {"hilfe", true},
    {"support", true},
};

void swap_frame(MarionetteClient* client, nlohmann::json& cframe, const nlohmann::json& frame) {
    if(cframe == frame) return;
    cframe = frame;
    client->switch_to_frame(frame);
}

#define NODE_ERR(t) ERROR("{} Node: {}, Tree: {}", t, node.dump(), tree.dump())
bool Parser::FormParser::handle_form_tree(MarionetteClient* client, nlohmann::json& tree, FormCache& question_cache)
{
    std::future<void> handle_captchas_done;
    assert_em(handle_captchas(client, handle_captchas_done), "handle_captchas() failed.");

    std::vector<nlohmann::json> new_cache;
    int l_clickable = -1;
    bool save_button = true;
    nlohmann::json cframe = nullptr;

    for(int i = 0; i < tree.size(); i++) {
        auto& node = tree[i];
        auto& type = node["type"];
        if(type == "txt") continue;
        std::string node_id = node["node"].front();
        nlohmann::json frame = node.contains("frame") ? node["frame"] : nullptr;

        bool skip = vector_contains(question_cache.tree_cache, node["node"]);
        if(skip) new_cache.push_back(node["node"]);
        if(type != "btn") {
            save_button = true;
            l_clickable = -1;
        }

        if(type == "ti") {
            if(skip) continue;
            std::string response;
            std::string text;

            if(!get_text(tree, i, text)) {
                ERROR("Failed find title. Node: {}, Tree: {}", node_id, tree.dump());
                continue;
            }

            StaticResponse::get_text_response(text, response, question_cache.context);

            swap_frame(client, cframe, frame);
            if(!client->execute_script(
                "arguments[0].value=arguments[1];arguments[0].dispatchEvent(new Event('change'));arguments[0].dispatchEvent(new Event('input'));",
                    nlohmann::json::array({node["node"], response})).get().success) {
                 NODE_ERR("Failed to set value of text input");
                 continue;
            }

            new_cache.push_back(node["node"]);

            continue;
        }

        if(type == "number") {
            if(skip) continue;
            std::string response;
            std::string text;

            int min = node["min"].is_null() ? INT_MIN : (int)node["min"];
            int max = node["max"].is_null() ? INT_MAX : (int)node["max"];
            int step = node["step"].is_null() ? 1 : (int)node["step"];

            if(!get_text(tree, i, text)) {
                ERROR("Failed find title. Node: {}, Tree: {}", node_id, tree.dump());
                continue;
            }

            StaticResponse::get_number_response(text, response, min, max, step, question_cache.context);

            swap_frame(client, cframe, frame);
            if(!client->execute_script(
                "arguments[0].value=arguments[1];arguments[0].dispatchEvent(new Event('change'));arguments[0].dispatchEvent(new Event('input'));",
                    nlohmann::json::array({node["node"], response})).get().success) {
                NODE_ERR("Failed to set value of number.");
                continue;
            }
            new_cache.push_back(node["node"]);

            continue;
        }

        if(type == "date") {
            if(skip) continue;
            if(!client->execute_script("arguments[0].value=\"1999-04-04\";arguments[0].dispatchEvent(new Event('change'));arguments[0].dispatchEvent(new Event('input'));", nlohmann::json::array({node["node"]})).get().success) {
                NODE_ERR("Failed to set value of date.");
                continue;
            }
            new_cache.push_back(node["node"]);

            continue;
        }

        if(type == "mc") {
            if(skip) continue;
            std::vector<int> response;
            std::string text;

            nlohmann::json& options_json = node["options"];
            if(options_json.size() == 0) {
                NODE_ERR("Multiple choice question has no options.");
                continue;
            }

            std::vector<std::string> options(options_json.size());
            for(int j = 0; j < options.size(); j++) {
                options[j] = options_json[j]["text"];
            }

            if(!get_text(tree, i, text)) {
                ERROR("Failed find title. Node: {}, Tree: {}", node_id, tree.dump());
                continue;
            }

            StaticResponse::get_multiple_choice_response(text, options, response, question_cache.context);

            swap_frame(client, cframe, frame);
            if(!client->execute_script(
                "arguments[0].selectedIndex=arguments[1];arguments[0].dispatchEvent(new Event('change'));",
                                       nlohmann::json::array({node["node"], response[0]})).get().success) {

                NODE_ERR("Failed to set value of select.");
                continue;
            }
            new_cache.push_back(node["node"]);
            continue;
        }

        if(type == "btn" && node.contains("glength")) {
            std::vector<int> response;
            std::string text;

            if(!get_prev_text_all(tree, i, text, false) && !find_title_js(client, node["hs"].front(), text)) {
                ERROR("Failed find title. Node: {}, Tree: {}", node_id, tree.dump());
                continue;
            }

            int glength = node["glength"];
            std::vector<std::string> options_str(glength);
            std::vector<nlohmann::json> options_node(glength);

            int option_count = 0;
            for(; i < tree.size(); i++) {
                if(tree[i]["type"] != "btn") continue;

                options_str[option_count] = tree[i]["text"];
                options_node[option_count] = tree[i]["node"];

                std::string txt_lower = options_str[option_count];
                std::transform(txt_lower.begin(), txt_lower.end(), txt_lower.begin(), tolower);
                for(auto& t : reject_text) {
                    if(!t.check_options) continue;
                    if(t.require_equals ? t.text != txt_lower : (txt_lower.find(t.text) == std::string::npos)) continue;
                    skip = true;
                    break;
                }

                option_count++;

                if(option_count == glength) break;
            }
            if(option_count < 2 || option_count != glength) {
                ERROR("Button group is missing options. Tree: {}, glength: {}, option_count: {}", tree.dump(), glength, option_count);
                continue;
            }

            if(!skip) {

                StaticResponse::get_multiple_choice_response(text, options_str, response, question_cache.context);

                swap_frame(client, cframe, frame);
                int clicked = 0;
                for(auto& opt : response) {
                    if(opt == -1) continue;
                    set_mouse_random(client);
                    if(element_click_mouse_move(client, options_node[opt].front()) || js_click_element(client, options_node[opt])) {
                        clicked++;
                        sleep_ms(400);
                        continue;
                    }
                    ERROR("Failed to click option. Node: {}, Text: {}, Tree: {}", options_node[opt].dump(), options_str[opt], tree.dump());
                    break;
                }
                if(clicked == 0) continue;
                new_cache.push_back(node["node"]);

                save_button = true;
                l_clickable = -1;
            }

            continue;
        }

        if(type == "btn" && (node["text"] == "Play" || node.contains("checkbox"))) {
            if(skip) continue;
            if(js_click_element(client, node["node"])) {
                new_cache.push_back(node["node"]);
                l_clickable = -1;
                break;
            }
            continue;
        }
        if(skip) continue;
        if(!save_button) continue;
        std::string btn_text = node["text"].is_null() ? "" : node["text"];
        std::transform(btn_text.begin(), btn_text.end(), btn_text.begin(), tolower);
        bool rejected = false;
        for(auto& t : reject_text) {
            if(t.require_equals ? t.text != btn_text : (btn_text.find(t.text) == std::string::npos)) continue;
            rejected = true;
            break;
        }
        if(rejected) continue;

        l_clickable = i;
        save_button = skip;
        continue;
    }

    handle_captchas_done.wait();
    sleep_ms(400);

    if(l_clickable != -1) swap_frame(client, cframe, tree[l_clickable].contains("frame") ? tree[l_clickable]["frame"] : nullptr);
    if(l_clickable != -1 && (element_click_mouse_move(client, tree[l_clickable]["node"].front()) || js_click_element(client, tree[l_clickable]["node"]))) {
        new_cache.push_back(tree[l_clickable]["node"]);
    }

    swap_frame(client, cframe, nullptr);

    if(question_cache.tree_cache == new_cache) question_cache.unchanged++;

    if(question_cache.unchanged >= 2) {
        question_cache.tree_cache.clear();
        question_cache.unchanged = 0;
        return true;
    }
    question_cache.tree_cache = new_cache;
    return true;
}

bool Parser::SpectrumSurveyParser::handle_form_tree(MarionetteClient* client, nlohmann::json& tree, FormCache& question_cache)
{
    if(!client->execute_script("document.querySelector(\"#submit-button\").click();", nlohmann::json::array({})).get().success) {
        ERROR("Spectrumsurvey submit failed.");
        return false;
    }
    return true;
}

