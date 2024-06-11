#include "util.h"
#include <signal.h>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <codecvt>
#include <locale>
#include <map>

Logger::Logger logger;

Logger::Logger &get_logger() {
    return logger;
}

std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string& get_alphabet() {
    return alphabet;
}



void set_signal_handlers(void (*func)(int)) {
    signal(SIGABRT, func);
    signal(SIGFPE, func);
    signal(SIGILL, func);
    signal(SIGINT, func);
    signal(SIGSEGV, func);
    signal(SIGTERM, func);
}

void reset_signal_handlers() {
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

long now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

double rand_multiplier() {
    srand(now());
    return ((double)rand())/RAND_MAX;
}

bool Timer::passed(long duration) {
    return time+duration < now();
}

bool Timer::passed() {
    return time < now();
}

void Timer::reset() {
    time = now();
}

Timer::Timer() {
    reset();
}

Timer::Timer(long timeout) {
    reset();
    time+=timeout;
}

void sleep_ms(long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

char num[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int is_num(char c) {
    for (int i = 0; i < 10; ++i) {
        if(c==num[i]) return i;
    }
    return -1;
}

void split(const std::string& in, const std::string& c, std::vector<std::string>& out)
{
    std::string tmp = in;
    size_t pos;
    while((pos = tmp.find(c)) != std::string::npos) {
        out.push_back(tmp.substr(0, pos));
        tmp = tmp.substr(pos+c.length(), tmp.length());
    }
    out.push_back(tmp);
}

void replace(std::string& str, const std::string& to_replace, const std::string& target, int cpos)
{
    size_t pos = str.find(to_replace, cpos);
    if(pos == std::string::npos) return;

    str.replace(pos, to_replace.length(), target);
    cpos = pos+target.length();
    replace(str, to_replace, target, cpos);
}

void rm_umlaut(std::string& str)
{
    replace(str, "Ä", "Ae");
    replace(str, "ä", "ae");
    replace(str, "Ö", "Oe");
    replace(str, "ö", "oe");
    replace(str, "Ü", "Ue");
    replace(str, "ü", "ue");
}

std::map<std::string, int> num_map = {
{"zero", 0},
{"one", 1},
{"two", 2},
{"three", 3},
{"four", 4},
{"five", 5},
{"six", 6},
{"seven", 7},
{"eight", 8},
{"nine", 9},
{"ten", 10},
{"eleven", 11},
{"twelve", 12},
};

bool txt_to_number(std::string str, int& out)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    auto it = num_map.find(str);
    if(it == num_map.end()) return false;
    out = (*it).second;
    return true;
}



char get_first_alpha(const std::string& str) {
    auto it = str.begin();
    while(it != str.end()) {
        if(isalpha(*it)) return *it;
    }
    return '\0';
}

std::string to_utf8(std::wstring& wide_string)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(wide_string);
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}
