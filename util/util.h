#ifndef POLLBOT_UTIL_H
#define POLLBOT_UTIL_H
#include <iostream>

#define VERSION "1.0"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#include <vector>
#include "Logger.h"

Logger::Logger& get_logger();

#define TRACE get_logger().logger->trace
#define INFO get_logger().logger->info
#define WARN get_logger().logger->warn
#define ERROR get_logger().logger->error
#define CRITICAL get_logger().logger->critical
#define DEBUG get_logger().logger->debug

#define stop_ std::promise<void>().get_future().wait()
#define assert_(x) if(!(x)) return false
#define assert_em(x, y) do { if(!(x)) { ERROR(y); return false; } } while(0)

#define LCHAR(s) (lxb_char_t*)s.c_str(), s.size()

void set_signal_handlers(void (*func)(int));
void reset_signal_handlers();
double rand_multiplier();
std::string& get_alphabet();

class Timer {
public:
    long time;
    bool passed(long duration);
    bool passed();
    void reset();
    Timer();
    Timer(long timeout);
};

void sleep_ms(long ms);

template<class... T>
bool wait_until(bool (*func)(T...), long timeout, T... args) {
    Timer timer(timeout);
    long tick = timeout > 0 ? timeout/10 : 100;
    while (!func(args...)) {
        if(timeout > 0) if(timer.passed()) return false;
        sleep_ms(tick);
    }
    return true;
}

std::string to_utf8(std::wstring& wide_string);
void split(const std::string& in, const std::string& c, std::vector<std::string>& out);
char get_first_alpha(const std::string& str);
void replace(std::string& str, const std::string& to_replace, const std::string& target, int cpos = 0);
void rm_umlaut(std::string& str);
bool txt_to_number(std::string str, int& out);

template<class T, int I>
bool array_contains(T (&array)[I], T& t) {
    for (int i = 0; i < I; ++i) {
        if(array[i] == t) return true;
    }
    return false;
}

template<class T>
bool vector_contains(std::vector<T>& vector, const T& t) {
    for(T& i : vector) {
        if(i == t) return true;
    }
    return false;
}

template<class T>
bool get_numbers(const std::string& str, std::vector<T>& out) {
    for(int i = 0; i < str.length(); i++) {
        if(!isdigit(str.at(i))) continue;
        size_t processed;
        float f = std::stof(str.substr(i), &processed);
        out.push_back(f);
        i+=processed-1;
    }
    return !out.empty();
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data);

#endif //POLLBOT_UTIL_H
