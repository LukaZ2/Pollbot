#ifndef TRANSLATE_H
#define TRANSLATE_H
#include <string>

namespace Translate {

    std::string detect_language(const std::string& str, int& confidence);
    void translate(std::string& str, const std::string& lang_source, const std::string& lang_target);

}

#endif // TRANSLATE_H
