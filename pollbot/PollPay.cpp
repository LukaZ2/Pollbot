#include "PollPay.h"
#include <util/util.h>
#include <unistd.h>
#include <thread>
#include <util/boilerplate.h>

PollPay::PollPay(MarionetteClient *client) : Provider(client), balance(0) {

}

std::string PollPay::url() {
    return "https://web.pollpay.app/login";
}

bool PollPay::enter(std::string username, std::string password) {

    assert_(close_all(client));
    assert_(client->navigate(url()).get().success);

    while(true) {
        if(wait_until_url_contains(client, "surveys/surveys", 1000)) break;
        if(wait_until_element_exists(client, CSS_SEL(".login-providers"), 1000)) if(!login(username, password)) return false;
    }
    return true;
}

bool PollPay::login(std::string username, std::string password) {
    assert_(client->navigate(url()).get().success);
    assert_(set_mouse_random(client));
    assert_(wait_until_element_exists(client, CSS_SEL(".login-providers"), 10000));
    if(wait_until_element_exists(client, BUTTON_T("Agree"), 2000)) element_click_mouse_move(client, BUTTON_T("Agree"));
    tablist tabs = get_tabs(client);
    assert_(element_click_mouse_move(client, CSS_SEL("button.bb-button:nth-child(1)")));
    sleep(1);
    assert_(client->switch_to_window(get_new_tabs(client, tabs)[0]).get().success);
    assert_(login_google(username, password));
    assert_(wait_until_url_contains(client, "surveys/surveys", 15000));
    return true;
}

double PollPay::get_balance() {
    return balance;
}

bool PollPay::start_poll() {
    element_click_mouse_move(client, BUTTON_T("Don't remind me again"));

    std::string poll_element;
    assert_(wait_until_element_exists(client, std::make_shared<QueryTagName>("div", "survey-tile"), &poll_element, 10000));

    std::string poll_button = client->find_element_from_element(poll_element, mTagName, "button").get().body["value"].front();
    assert_(element_click_mouse_move(client, poll_button));
    sleep_ms(800);

    std::string start_survey_button;
    assert_(wait_until_element_exists(client, std::make_shared<QueryXPath>("//button//span[text()=\'Start Survey\']"), &start_survey_button, 10000));
    assert_(element_click_mouse_move(client, start_survey_button));
    return true;
}

bool PollPay::login_google(std::string& username, std::string& password) {
    assert_(element_click_mouse_move(client, CSS_SEL("#identifierId")));
    sleep_ms(400);
    assert_(element_send_keys(client, CSS_SEL("#identifierId"), username, 300));
    sleep_ms(400);
    assert_(element_click_mouse_move(client, CSS_SEL(".VfPpkd-LgbsSe-OWXEXe-k8QpJ")));
    sleep_ms(400);

    assert_(wait_until_element_exists(client, TAG_ATTR("input", "name", "Passwd"), 5000));
    assert_(element_click_mouse_move(client, TAG_ATTR("input", "name", "Passwd")));
    sleep_ms(400);
    assert_(element_send_keys(client, TAG_ATTR("input", "name", "Passwd"), password, 300));
    sleep_ms(400);
    assert_(element_click_mouse_move(client, CSS_SEL(".VfPpkd-LgbsSe-OWXEXe-k8QpJ")));
    return true;
}

bool PollPay::on_main_page(MarionetteClient* client)
{
    return get_url(client).find("pollpay.app/surveys/surveys") != -1;
}
