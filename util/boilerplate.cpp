#include "boilerplate.h"

#include <utility>
#include "util.h"

ElementQuery::ElementQuery() = default;
ElementQuery::~ElementQuery() = default;
std::string ElementQuery::find(MarionetteClient* client) {
    return {};
}

QueryTagName::QueryTagName(const std::string& tag) : tag(tag) {}
QueryTagName::QueryTagName(const std::string &tag, const std::string &css_class) : tag(tag), css_class(css_class) {}
std::string QueryTagName::find(MarionetteClient* client) {
    if(css_class.empty()) {
        mResponse response = client->find_element(mTagName, tag).get();
        if (!response.success) return {};
        return response.body["value"].front();
    }
    return QueryCSS(tag + "." + css_class).find(client);
}

QueryCSS::QueryCSS(const std::string &css) : css(css) {}
std::string QueryCSS::find(MarionetteClient* client) {
    mResponse response = client->find_element(mCSS_Selector, css).get();
    if(!response.success) return {};
    return response.body["value"].front();
}

QueryButtonText::QueryButtonText(const std::string &text) : text(text) {}
std::string QueryButtonText::find(MarionetteClient *client) {
    return QueryXPath("//button[contains(text(),\"" + text + "\")]").find(client);
}

QueryXPath::QueryXPath(const std::string &xpath) : xpath(xpath) {}
std::string QueryXPath::find(MarionetteClient *client) {
    mResponse response = client->find_element(mXPath, xpath).get();
    if(!response.success) return {};
    return response.body["value"].front();
}


tablist get_tabs(MarionetteClient* client) {
    mResponse tabs = client->get_window_handles().get();
    if(!tabs.success) return {};
    tablist result;
    for (int i = 0; i < tabs.body.size(); ++i) {
        result.push_back(tabs.body[i]);
    }
    return result;
}

tablist get_new_tabs(const tablist& current, const tablist& old)
{
    if(current.empty()) return {};
    tablist result;
    for (int i = 0; i < current.size(); ++i) {
        bool found = false;
        for (int j = 0; j < old.size(); ++j) {
            if((found = (current[i] == old[j]))) break;
        }
        if(!found) result.push_back(current[i]);
    }
    return result;
}

tablist get_new_tabs(MarionetteClient* client, const tablist& old) {
    return get_new_tabs(get_tabs(client), old);
}

std::string get_element_from_array_using_text(MarionetteClient* client, nlohmann::json array, std::string query) {
    std::string query_ = std::string("\"" + query + "\"");
    for (int i = 0; i < array.size(); ++i) {
        std::string text = to_string(client->get_element_text(array[i].front()).get().body["value"]);
        if(text == query_) return array[i].front();
    }
    return "";
}

std::string
get_element_from_array_using_attribute(MarionetteClient *client, nlohmann::json array, std::string attribute,
                                       std::string name) {
    std::string query_ = std::string("\"" + name + "\"");
    for (int i = 0; i < array.size(); ++i) {
        std::string attribute_ = to_string(client->get_element_attribute(array[i].front(), attribute).get().body["value"]);
        if(attribute_ == query_) return array[i].front();
    }
    return "";
}

std::string get_element(MarionetteClient *client, const std::shared_ptr<ElementQuery>& query) {
    return query->find(client);
}

bool set_mouse_random(MarionetteClient* client) {
    mResponse rect = client->get_window_rect().get();
    if(!rect.success) return false;
    int x = (rand_multiplier()*((double)rect.body["width"]));
    int y = (rand_multiplier()*((double)rect.body["height"]-100));
    return client->simple_set_mouse(x, y).get().success;
}

bool element_click_mouse_move(MarionetteClient* client, std::string id) {
    mResponse element_rect = client->get_element_rect(id).get();
    mResponse window_rect = client->get_window_rect().get();
    if(!element_rect.success) return false;
    if(!window_rect.success) return false;
    int x = ((double) element_rect.body["x"]) + (rand_multiplier() * ((double) element_rect.body["width"]));
    int y = ((double) element_rect.body["y"]) + (rand_multiplier() * ((double) element_rect.body["height"]));
    if(x >= (int)window_rect.body["width"]) x = ((int)window_rect.body["width"])-1;
    if(y >= (int)window_rect.body["height"]-100) y = ((int)window_rect.body["height"])-100;
    if(x <= 0) x = 1;
    if(y <= 0) y = 1;
    if(!client->set_mouse_action_queue({mAction(POINTER_MOVE, (rand_multiplier()*200)+200, x, y, false)}).get().success) return false;
    return client->element_click(id).get().success;
}

bool element_click_mouse_move(MarionetteClient* client, const std::shared_ptr<ElementQuery>& query) {
    return element_click_mouse_move(client, query->find(client));
}

bool element_send_keys(MarionetteClient *client, std::string id, std::string keys, long wait) {
    if(wait == -1) wait = 200;
    std::transform(keys.begin(), keys.end(), keys.begin(), [](wchar_t c)->char {
        switch(c) {
            case L'ü':
            case L'Ü':
                return 'u';
            case L'ö':
            case L'Ö':
                return 'o';
            case L'ä':
            case L'Ä':
                return 'a';
        }
        return c;
    });
    for (int i = 0; i < keys.length(); ++i) {
        std::string key;
        key += keys.at(i);
        assert_(client->element_send_keys(id, key).get().success);
        sleep_ms((wait*rand_multiplier())+(wait/2));
    }
    return true;
}

bool element_send_keys(MarionetteClient *client, const std::shared_ptr<ElementQuery>& query, std::string keys, long wait) {
    return element_send_keys(client, query->find(client), keys, wait);
}

bool element_exists(MarionetteClient *client, std::shared_ptr<ElementQuery> query, std::string* out) {
    if(out == nullptr) return !query->find(client).empty();
    *out = query->find(client);
    return !out->empty();
}

bool wait_until_element_exists(MarionetteClient* client, const std::shared_ptr< ElementQuery >& query, long int timeout) {
    return wait_until_element_exists(client, query, nullptr, timeout);
}

bool wait_until_element_exists(MarionetteClient *client, const std::shared_ptr<ElementQuery>& query, std::string* out, long timeout) {
    return wait_until(element_exists, timeout, client, query, out);
}

bool url_equals(long client, std::string url) {
    return ((MarionetteClient*)client)->get_current_url().get().body["value"] == std::string("\"" + url + "\"");
}

bool wait_until_url_equals(MarionetteClient *client, std::string url, long timeout) {
    return wait_until(url_equals, timeout, (long)client, url);
}

bool url_contains(long client, std::string url) {
    return to_string(((MarionetteClient*)client)->get_current_url().get().body["value"]).find(url) != std::string::npos;
}

bool wait_until_url_contains(MarionetteClient *client, std::string url, long timeout) {
    return wait_until(url_contains, timeout, (long)client, url);
}

bool tag_exists(long client, std::string tag) {
    return ((MarionetteClient*)client)->find_element(mTagName, tag).get().success;
}

bool wait_until_tag_exists(MarionetteClient *client, std::string tag, long timeout) {
    return wait_until(tag_exists, timeout, (long) client, tag);
}

bool wait_until_page_stable(MarionetteClient* client, int stable_length)
{
    mResponse response = client->get_page_source().get();
    assert_(response.success);
    int i = 0;
    while(true) {
        sleep_ms(1000);
        mResponse tmp = client->get_page_source().get();
        assert_(tmp.success);
        if(response.body == tmp.body) {
            i++;
            if(i >= stable_length) return true;
            continue;
        }
        i = 0;
        response = tmp;
    }
}


bool close_all(MarionetteClient* client) {

    tablist tabs = get_tabs(client);
    assert_(!tabs.empty());
    if(tabs.size() == 1) return true;
    for(int i = 0; i < tabs.size()-1; i++) {
        assert_(client->switch_to_window(tabs[i]).get().success);
        assert_(client->close_window().get().success);
    }
    assert_(client->switch_to_window(tabs[tabs.size()-1]).get().success);
    assert_(client->maximize_window().get().success);
    return true;
}

std::string get_url(MarionetteClient* client) {
    mResponse response = client->get_current_url().get();
    if(!response.success) return "";
    return response.body["value"];
}

MarionettePacket::Response execute_js_file(MarionetteClient* client, std::string file, nlohmann::json args)
{
    std::stringstream ss;
#ifndef NDEBUG
    file.insert(0, "../");
#endif
    std::ifstream f(file);
    ss << f.rdbuf();
    std::string script = ss.str();
    f.close();
    return client->execute_script(script, args).get();
}

bool js_click_element(MarionetteClient* client, const nlohmann::json& element)
{
    return client->execute_script("arguments[0].click();", nlohmann::json::array({element})).get().success;
}
