#include "localizer.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace i18n {

std::unordered_map<std::string, std::string> Localizer::s_translations;
std::string Localizer::s_langCode = "en_US";

static const char* idKeys[] = {
    "TITLE_MAIN",
    "TITLE_CONFIG",
    "TITLE_PICKING",
    "NAME_LABEL",
    "REMAINING_LABEL",
    "PERSON_UNIT",
    "INVALID_INPUT",
    "PRESS_ENTER",
    "LIST_EMPTY",
    "CONTINUE_ASK",
    "PREPARING",
    "CURRENT_MODE",
    "NAME_COUNT",
    "SETUP_HINT",
    "PLEASE_WAIT",
    "END",
    "UNABLE_OPEN_CONFIG",
    "UNABLE_WRITE_CONFIG",
    "MODE_ALL_RANDOM",
    "MODE_ONE_BY_ONE",
    "UNABLE_OPEN_LIST",
    "CURRENT_LIST",
    "NAMES_COUNT",
    "ADD_NAME",
    "CLEAR_LIST",
    "SAVE_RETURN",
    "PLEASE_CHOOSE",
    "ENTER_NAME",
    "LIST_CLEARED",
    "LIST_EDITED",
    "EDIT_LIST",
    "SET_MODE",
    "SAVE_EXIT",
    "CHOOSE_MODE",
    "MODE_1_ALL",
    "MODE_2_ONE",
    "SETTINGS_SAVED",
    "TUI_HELP",
    "TUI_HELP_TITLE",
    "TUI_HELP_NEXT",
    "TUI_HELP_HIDE",
    "TUI_HELP_MODE",
    "TUI_HELP_LANG",
    "TUI_HELP_RESTART",
    "TUI_HELP_SETUP",
    "TUI_HELP_QUIT",
    "TUI_HELP_HELP",
    "TUI_HELP_BACK",
    "TUI_HELP_SHELL",
    "TUI_HELP_OP",
    "TUI_HELP_ABOUT",
    "ABOUT_TITLE",
    "ABOUT_PROJECT",
    "ABOUT_DESC",
    "ABOUT_AUTHOR",
    "TUI_DONE",
    "TUI_EMPTY",
    "TUI_HIDDEN",
    "TUI_NEXT_LABEL",
    "TUI_HINT_DONE",
    "TUI_HINT_EXIT",
    "SETUP_HINT_LIST",
    "SETUP_HINT_SAVE",
    "SETUP_HINT_ADD",
    "SETUP_HINT_EDIT",
    "SETUP_LIST_EDIT",
};

std::string Localizer::idToKey(ID id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(ID::_COUNT)) return "";
    return idKeys[idx];
}

std::string Localizer::unescapeXML(const std::string& text) {
    std::string result = text;
    auto replace = [&](const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
    };
    replace("&amp;", "&");
    replace("&lt;", "<");
    replace("&gt;", ">");
    replace("&quot;", "\"");
    replace("&apos;", "'");
    return result;
}

bool Localizer::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    // Find Console context
    std::string ctxTag = "<name>Console</name>";
    size_t ctxStart = content.find(ctxTag);
    if (ctxStart == std::string::npos) return false;

    size_t ctxOpen = content.rfind("<context", ctxStart);
    if (ctxOpen == std::string::npos) return false;

    size_t ctxClose = content.find("</context>", ctxOpen);
    if (ctxClose == std::string::npos) return false;

    std::string ctxSection = content.substr(ctxOpen, ctxClose - ctxOpen + 10);

    // Parse <message> entries
    size_t pos = 0;
    while (true) {
        size_t msgStart = ctxSection.find("<message>", pos);
        if (msgStart == std::string::npos) break;
        size_t msgEnd = ctxSection.find("</message>", msgStart);
        if (msgEnd == std::string::npos) break;

        std::string msg = ctxSection.substr(msgStart, msgEnd - msgStart + 10);
        pos = msgEnd + 10;

        // Extract <source>
        std::string source;
        size_t s = msg.find("<source>");
        if (s == std::string::npos) continue;
        s += 8;
        size_t e = msg.find("</source>", s);
        if (e == std::string::npos) continue;
        source = unescapeXML(msg.substr(s, e - s));

        // Extract <translation...>
        std::string trans;
        s = msg.find("<translation");
        if (s == std::string::npos) continue;
        s = msg.find('>', s) + 1;
        e = msg.find("</translation>", s);
        if (e == std::string::npos) continue;
        trans = unescapeXML(msg.substr(s, e - s));

        if (!source.empty()) {
            if (!trans.empty()) {
                s_translations[source] = trans;
            } else if (s_translations.find(source) == s_translations.end()) {
                s_translations[source] = source;
            }
        }
    }

    return true;
}

void Localizer::setLanguage(const std::string& langCode) {
    s_langCode = langCode;
    s_translations.clear();

    loadFile("trans/RandomNamePicker_en_US.ts");

    if (langCode != "en_US") {
        std::string path = "trans/RandomNamePicker_" + langCode + ".ts";
        loadFile(path);
    }
}

Language Localizer::getLanguage() {
    return parseLanguage(s_langCode);
}

std::string Localizer::currentLanguageCode() {
    return s_langCode;
}

std::string Localizer::get(ID id) {
    std::string key = idToKey(id);
    if (key.empty()) return "";
    auto it = s_translations.find(key);
    if (it != s_translations.end()) return it->second;
    return key;
}

std::string Localizer::get(const std::string& key) {
    auto it = s_translations.find(key);
    if (it != s_translations.end()) return it->second;
    return key;
}

Language Localizer::parseLanguage(const std::string& langStr) {
    if (langStr == "zh-CN" || langStr == "zh_CN" || langStr == "zh") {
        return Language::ZH_CN;
    }
    return Language::EN_US;
}

std::string Localizer::languageToString(Language lang) {
    switch (lang) {
        case Language::ZH_CN:
            return "zh_CN";
        case Language::EN_US:
        default:
            return "en_US";
    }
}

}
