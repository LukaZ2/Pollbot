#include "MarionetteClient.h"
#include "../util/util.h"

MarionetteClient::Action::Action(const std::string &type, int duration, int x, int y, bool fromOrigin) :
        type(type),duration(duration),x(x), y(y),from_origin(fromOrigin) {}

bool MarionetteClient::inject_jquery()
{
    return execute_script("js/jquery.js", {}).get().success;
}


fResponse MarionetteClient::new_session() {
    nlohmann::json body = {{"capabilities", {}}};
    return queue_packet("WebDriver:NewSession", body);
}

fResponse MarionetteClient::navigate(const std::string& url) {
    return queue_packet("WebDriver:Navigate", {{"url", url}});
}

fResponse MarionetteClient::get_current_url() {
    return queue_packet("WebDriver:GetCurrentURL", {});
}

fResponse MarionetteClient::get_current_title() {
    return queue_packet("WebDriver:GetTitle", {});
}

fResponse MarionetteClient::get_window_handles() {
    return queue_packet("WebDriver:GetWindowHandles", {});
}

fResponse MarionetteClient::switch_to_window(const std::string& handle) {
    return queue_packet("WebDriver:SwitchToWindow", {{"handle", handle}});
}

fResponse MarionetteClient::take_screenshot() {
    return queue_packet("WebDriver:TakeScreenshot", {});
}

fResponse MarionetteClient::dismiss_alert() {
    return queue_packet("WebDriver:DismissAlert", {});
}

fResponse MarionetteClient::simple_set_mouse(int x, int y) {
    nlohmann::json actions = nlohmann::json::array({
        {{"type", "pointer"}, {"id", "cursor"}, {"actions", nlohmann::json::array({
            {{"type", "pointerMove"}, {"x", x}, {"y", y}}
        })}}
    });
    return queue_packet("WebDriver:PerformActions", {{"actions", actions}});
}

fResponse MarionetteClient::set_mouse_action_queue(const std::vector<Action>& actions) {
    nlohmann::json source;
    source["type"] = "pointer";
    source["id"] = "cursor";

    nlohmann::json source_actions;
    for (int i = 0; i < actions.size(); ++i) {
        Action action = actions[i];
        source_actions[i]["type"] = action.type;
        if(action.x != -1) {
            source_actions[i]["x"] = action.x;
            source_actions[i]["y"] = action.y;
            if(action.from_origin) source_actions[i]["origin"] = "pointer";
        }
        source_actions[i]["duration"] = action.duration;
    }
    source["actions"] = source_actions;
    return queue_packet("WebDriver:PerformActions", {{"actions", {source}}});
}

fResponse MarionetteClient::find_element(const std::string& method, const std::string& value) {
    return queue_packet("WebDriver:FindElement", {{"using", method}, {"value", value}});
}

fResponse MarionetteClient::find_elements(const std::string& method, const std::string& value) {
    return queue_packet("WebDriver:FindElements", {{"using", method}, {"value", value}});
}

fResponse MarionetteClient::get_page_source() {
    return queue_packet("WebDriver:GetPageSource", {});
}

fResponse MarionetteClient::element_click(const std::string& id) {
    return queue_packet("WebDriver:ElementClick", {{"id", id}});
}

fResponse MarionetteClient::get_element_text(const std::string& id) {
    return queue_packet("WebDriver:GetElementText", {{"id", id}});
}

fResponse MarionetteClient::get_window_rect() {
    return queue_packet("WebDriver:GetWindowRect", {});
}

fResponse MarionetteClient::get_element_rect(const std::string& id) {
    return queue_packet("WebDriver:GetElementRect", {{"id", id}});
}

fResponse MarionetteClient::element_send_keys(const std::string& elementId, const std::string& text) {
    return queue_packet("WebDriver:ElementSendKeys", {{"id", elementId}, {"text", text}});
}

fResponse MarionetteClient::get_element_attribute(const std::string& id, const std::string& attribute) {
    return queue_packet("WebDriver:GetElementAttribute", {{"id", id}, {"name", attribute}});
}

fResponse MarionetteClient::find_element_from_element(const std::string& elementId, const std::string& method, const std::string& value) {
    return queue_packet("WebDriver:FindElement", {{"id", elementId}, {"using", method}, {"value", value}});
}

fResponse MarionetteClient::find_elements_from_element(const std::string& elementId, const std::string& method, const std::string& value) {
    return queue_packet("WebDriver:FindElements", {{"id", elementId}, {"using", method}, {"value", value}});
}

fResponse MarionetteClient::execute_async_script(const std::string& script, const nlohmann::json& args) {
    return queue_packet("WebDriver:ExecuteAsyncScript", {{"script", script}, {"args", args}});
}

fResponse MarionetteClient::execute_script(const std::string& script, const nlohmann::json& args) {
    return queue_packet("WebDriver:ExecuteScript", {{"script", script}, {"args", args}});
}

fResponse MarionetteClient::close_window() {
    return queue_packet("WebDriver:CloseWindow", {});
}

fResponse MarionetteClient::maximize_window() {
    return queue_packet("WebDriver:MaximizeWindow", {});
}

fResponse MarionetteClient::get_element_tag_name(const std::string& id)
{
    return queue_packet("WebDriver:GetElementTagName", {{"id", id}});
}
