#include "Logger.h"
#include <filesystem>
#include <chrono>
#include "../ui/Console.h"
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <mutex>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/ansicolor_sink-inl.h>
#include <spdlog/pattern_formatter.h>
#include <readline/readline.h>

using namespace std;
namespace fs = filesystem;

void rl_sync_print(char *input)
{
    int should_refresh = (rl_readline_state & RL_STATE_READCMD) > 0;
    char *saved_line;
    int saved_point;
    if (should_refresh)
    {
        saved_point = rl_point;
        saved_line = rl_copy_text(0, rl_end);
        rl_save_prompt();
        rl_replace_line("", 0);
        rl_redisplay();
    }
    printf(input);

    if (should_refresh)
    {
        rl_restore_prompt();
        rl_replace_line(saved_line, 0);
        rl_point = saved_point;
        rl_redisplay();
        free(saved_line);
    }
}

template<class Mutex>
class rl_sync_sink : public spdlog::sinks::base_sink<Mutex> {
public:

    // Formatting codes
    const spdlog::string_view_t reset = "\033[m";
    const spdlog::string_view_t bold = "\033[1m";
    const spdlog::string_view_t dark = "\033[2m";
    const spdlog::string_view_t underline = "\033[4m";
    const spdlog::string_view_t blink = "\033[5m";
    const spdlog::string_view_t reverse = "\033[7m";
    const spdlog::string_view_t concealed = "\033[8m";
    const spdlog::string_view_t clear_line = "\033[K";

    // Foreground colors
    const spdlog::string_view_t black = "\033[30m";
    const spdlog::string_view_t red = "\033[31m";
    const spdlog::string_view_t green = "\033[32m";
    const spdlog::string_view_t yellow = "\033[33m";
    const spdlog::string_view_t blue = "\033[34m";
    const spdlog::string_view_t magenta = "\033[35m";
    const spdlog::string_view_t cyan = "\033[36m";
    const spdlog::string_view_t white = "\033[37m";

    /// Background colors
    const spdlog::string_view_t on_black = "\033[40m";
    const spdlog::string_view_t on_red = "\033[41m";
    const spdlog::string_view_t on_green = "\033[42m";
    const spdlog::string_view_t on_yellow = "\033[43m";
    const spdlog::string_view_t on_blue = "\033[44m";
    const spdlog::string_view_t on_magenta = "\033[45m";
    const spdlog::string_view_t on_cyan = "\033[46m";
    const spdlog::string_view_t on_white = "\033[47m";

    /// Bold colors
    const spdlog::string_view_t yellow_bold = "\033[33m\033[1m";
    const spdlog::string_view_t red_bold = "\033[31m\033[1m";
    const spdlog::string_view_t bold_on_red = "\033[1m\033[41m";

    Mutex mutex_;

    std::array<std::string, spdlog::level::n_levels> colors_;
    std::stringstream buffer;

    rl_sync_sink() {
        colors_[spdlog::level::trace] = to_string_(white);
        colors_[spdlog::level::debug] = to_string_(cyan);
        colors_[spdlog::level::info] = to_string_(green);
        colors_[spdlog::level::warn] = to_string_(yellow_bold);
        colors_[spdlog::level::err] = to_string_(red_bold);
        colors_[spdlog::level::critical] = to_string_(bold_on_red);
        colors_[spdlog::level::off] = to_string_(reset);
    }
protected:

    char* saved_line = nullptr;
    int saved_point = 0;
    int should_refresh;

    std::string to_string_(const spdlog::string_view_t &sv)
    {
        return std::string(sv.data(), sv.size());
    }

    void print_ccode_(const spdlog::string_view_t &color_code)
    {
        fwrite(color_code.data(), sizeof(char), color_code.size(), stdout);
    }

    void print_range_(const spdlog::memory_buf_t &formatted, size_t start, size_t end)
    {
        fwrite(formatted.data() + start, sizeof(char), end - start, stdout);
    }

    void save_line() {
        should_refresh = (rl_readline_state & RL_STATE_READCMD) > 0;
        if (should_refresh)
        {
            saved_point = rl_point;
            saved_line = rl_copy_text(0, rl_end);
            rl_save_prompt();
            rl_replace_line("", 0);
            rl_redisplay();
        }
    }
    void restore_line() {
        if (should_refresh)
        {
            rl_restore_prompt();
            rl_replace_line(saved_line, 0);
            rl_point = saved_point;
            rl_redisplay();
            free(saved_line);
        }
    }

    void sink_it_(const spdlog::details::log_msg & msg) override {
        std::lock_guard<Mutex> lock(mutex_);
        msg.color_range_start = 0;
        msg.color_range_end = 0;
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        save_line();
        // before color range
        print_range_(formatted, 0, msg.color_range_start);
        // in color range
        print_ccode_(colors_[static_cast<size_t>(msg.level)]);
        print_range_(formatted, msg.color_range_start, msg.color_range_end);
        print_ccode_(reset);
        // after color range
        print_range_(formatted, msg.color_range_end, formatted.size());
        fflush(stdout);

        restore_line();
    }
    void flush_() override {
        fflush(stdout);
    }
};

using rl_sync_sink_mt = rl_sync_sink<std::mutex>;

Logger::Logger::Logger() {

    if(!fs::exists("logs")) fs::create_directory("logs");
    if(fs::exists("logs/latest.log")) fs::rename("logs/latest.log", "logs/" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log");

    spdlog::init_thread_pool(8192, 1);

    auto console_sink = make_shared<rl_sync_sink_mt>();
    auto file_sink = make_shared<spdlog::sinks::basic_file_sink_mt>("logs/latest.log");

    file_sink->set_pattern("[%T] [T%t] [%^%l%$] %v");
    console_sink->set_pattern("[%T] [T%t] [%^%l%$] %v");

    sinks = {console_sink, file_sink};
    logger = std::make_shared<spdlog::async_logger>("logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
}
