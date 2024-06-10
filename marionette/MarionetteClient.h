#ifndef POLLBOT_MARIONETTECLIENT_H
#define POLLBOT_MARIONETTECLIENT_H
#include "BaseMarionetteClient.h"
#include <vector>

#define fResponse std::future<MarionettePacket::Response>
#define mResponse MarionettePacket::Response

#define mAction MarionetteClient::Action
#define mActionPause(duration) MarionetteClient::Action("pause", duration, -1, -1, false)
#define POINTER_MOVE "pointerMove"

#define mCSS_Selector "css selector"
#define mLinkText "link text"
#define mPartialLinkText "partial link text"
#define mTagName "tag name"
#define mXPath "xpath"

class MarionetteClient : public BaseMarionetteClient {
public:
    typedef std::shared_ptr<MarionetteClient> ptr;

    struct Action {
        std::string type;
        int duration;
        int x;
        int y;
        bool from_origin;
        Action(const std::string &type, int duration, int x, int y, bool fromOrigin);
    };

    bool inject_jquery();

    fResponse new_session();
    fResponse navigate(const std::string& url);
    fResponse get_current_url();
    fResponse get_current_title();
    fResponse get_window_handles();
    fResponse switch_to_window(const std::string& handle);
    fResponse switch_to_frame(const nlohmann::json& element);
    fResponse switch_to_frame(int index);
    fResponse switch_to_origin_frame();
    fResponse take_screenshot();
    fResponse dismiss_alert();
    fResponse simple_set_mouse(int x, int y);
    fResponse set_mouse_action_queue(const std::vector<Action>& actions);
    fResponse find_element(const std::string& method, const std::string& value);
    fResponse find_elements(const std::string& method, const std::string& value);
    fResponse get_page_source();
    fResponse element_click(const std::string& id);
    fResponse get_element_text(const std::string& id);
    fResponse get_window_rect();
    fResponse get_element_rect(const std::string& id);
    fResponse element_send_keys(const std::string& elementId, const std::string& text);
    fResponse get_element_attribute(const std::string& id, const std::string& attribute);
    fResponse find_element_from_element(const std::string& elementId, const std::string& method, const std::string& value);
    fResponse find_elements_from_element(const std::string& elementId, const std::string& method, const std::string& value);
    fResponse execute_async_script(const std::string& script, const nlohmann::json& args);
    fResponse execute_script(const std::string& script, const nlohmann::json& args);
    fResponse close_window();
    fResponse maximize_window();
    fResponse get_element_tag_name(const std::string& id);
};

#endif //POLLBOT_MARIONETTECLIENT_H
