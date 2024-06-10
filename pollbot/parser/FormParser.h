#ifndef FORMPARSER_H
#define FORMPARSER_H
#include <string>
#include <memory>
#include <marionette/MarionetteClient.h>
#include <future>
#include <thread>

namespace Parser {

    class FormCache {
    public:
        std::vector<nlohmann::json> tree_cache;
        nlohmann::json context;
    };

    class FormParser {
    public:
        typedef std::shared_ptr<FormParser> ptr;

        virtual ~FormParser() = default;

        bool get_form_tree(MarionetteClient* client, nlohmann::json& tree);
        bool has_interactables(const nlohmann::json& tree);
        bool handle_captchas(MarionetteClient* client, std::future<void>& done);

        virtual bool handle_form_tree(MarionetteClient* client, nlohmann::json& tree, FormCache& question_cache);
    };

    class SpectrumSurveyParser : public FormParser {
    public:
        bool handle_form_tree(MarionetteClient * client, nlohmann::json & tree, FormCache& question_cache) override;
    };

    FormParser* get_parser(const std::string& url);
}

#endif // FORMPARSER_H
