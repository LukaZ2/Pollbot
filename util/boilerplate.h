#ifndef POLLBOT_BOILERPLATE_H
#define POLLBOT_BOILERPLATE_H
#include <string>
#include "../marionette/MarionetteClient.h"
#include "json.hpp"

class ElementQuery {
public:
    explicit ElementQuery();
    virtual ~ElementQuery();
    virtual std::string find(MarionetteClient* client);
};

class QueryTagName : public ElementQuery {
public:
    const std::string tag;
    const std::string css_class;
    explicit QueryTagName(const std::string& tag);
    explicit QueryTagName(const std::string& tag, const std::string& css_class);
    std::string find(MarionetteClient* client) override;
};

#define CSS_SEL(x) std::make_shared<QueryCSS>(x)
class QueryCSS : public ElementQuery {
public:
    const std::string css;
    explicit QueryCSS(const std::string &css);
    std::string find(MarionetteClient* client) override;
};

#define XPATH_SEL(x) std::make_shared<QueryXPath>(x)
class QueryXPath : public ElementQuery {
public:
    const std::string xpath;
    explicit QueryXPath(const std::string &xpath);
    std::string find(MarionetteClient *client) override;
};

#define BUTTON_T(x) std::make_shared<QueryButtonText>(x)
class QueryButtonText : public ElementQuery {
public:
    const std::string text;
    explicit QueryButtonText(const std::string &text);
    std::string find(MarionetteClient *client) override;
};

#define TAG_ATTR(tag, key, val) std::make_shared<QueryXPath>(std::string("//") + tag + "[@" + key + "=\'" + val + "\']")

typedef std::vector<std::string> tablist;
tablist get_tabs(MarionetteClient* client);
tablist get_new_tabs(MarionetteClient* client, const tablist& old);

std::string get_element_from_array_using_text(MarionetteClient* client, nlohmann::json array, std::string query);
std::string get_element_from_array_using_attribute(MarionetteClient* client, nlohmann::json array, std::string attribute, std::string name);
bool set_mouse_random(MarionetteClient* client);
bool element_click_mouse_move(MarionetteClient* client, std::string id);
bool element_click_mouse_move(MarionetteClient* client, const std::shared_ptr<ElementQuery>& query);
bool element_send_keys(MarionetteClient* client, const std::shared_ptr<ElementQuery>& query, std::string keys, long wait = -1);
bool element_send_keys(MarionetteClient* client, std::string id, std::string keys, long wait = -1);
bool element_exists(MarionetteClient *client, const std::shared_ptr<ElementQuery>& query, std::string* out);
bool wait_until_element_exists(MarionetteClient *client, const std::shared_ptr<ElementQuery>& query, long timeout);
bool wait_until_element_exists(MarionetteClient *client, const std::shared_ptr<ElementQuery>& query, std::string* out, long timeout);
bool wait_until_url_equals(MarionetteClient* client, std::string url, long timeout);
bool wait_until_url_contains(MarionetteClient* client, std::string url, long timeout);
bool wait_until_tag_exists(MarionetteClient* client, std::string tag, long timeout);
bool wait_until_page_stable(MarionetteClient* client, int stable_length);

bool close_all(MarionetteClient* client);

std::string get_url(MarionetteClient* client);
mResponse execute_js_file(MarionetteClient* client, std::string file, nlohmann::json args = nlohmann::json::array({}));
bool js_click_element(MarionetteClient* client, const nlohmann::json& element);

#endif //POLLBOT_BOILERPLATE_H
