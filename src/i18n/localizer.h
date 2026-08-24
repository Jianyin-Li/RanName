#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <string>
#include <unordered_map>

namespace i18n {

enum class ID {
    TITLE_MAIN,
    TITLE_CONFIG,
    TITLE_PICKING,
    NAME_LABEL,
    REMAINING_LABEL,
    PERSON_UNIT,
    INVALID_INPUT,
    PRESS_ENTER,
    LIST_EMPTY,
    CONTINUE_ASK,
    PREPARING,
    CURRENT_MODE,
    NAME_COUNT,
    SETUP_HINT,
    PLEASE_WAIT,
    END,
    UNABLE_OPEN_CONFIG,
    UNABLE_WRITE_CONFIG,
    MODE_ALL_RANDOM,
    MODE_ONE_BY_ONE,
    UNABLE_OPEN_LIST,
    CURRENT_LIST,
    NAMES_COUNT,
    ADD_NAME,
    CLEAR_LIST,
    SAVE_RETURN,
    PLEASE_CHOOSE,
    ENTER_NAME,
    LIST_CLEARED,
    LIST_EDITED,
    EDIT_LIST,
    SET_MODE,
    SAVE_EXIT,
    CHOOSE_MODE,
    MODE_1_ALL,
    MODE_2_ONE,
    SETTINGS_SAVED,
    TUI_HELP,
    TUI_HELP_TITLE,
    TUI_HELP_NEXT,
    TUI_HELP_HIDE,
    TUI_HELP_MODE,
    TUI_HELP_LANG,
    TUI_HELP_RESTART,
    TUI_HELP_SETUP,
    TUI_HELP_QUIT,
    TUI_HELP_HELP,
    TUI_HELP_BACK,
    TUI_HELP_SHELL,
    TUI_HELP_OP,
    TUI_HELP_ABOUT,
    ABOUT_TITLE,
    ABOUT_PROJECT,
    ABOUT_DESC,
    ABOUT_AUTHOR,
    TUI_DONE,
    TUI_EMPTY,
    TUI_HIDDEN,
    TUI_NEXT_LABEL,
    TUI_HINT_DONE,
    TUI_HINT_EXIT,
    SETUP_HINT_LIST,
    SETUP_HINT_SAVE,
    SETUP_HINT_ADD,
    SETUP_HINT_EDIT,
    SETUP_LIST_EDIT,
    _COUNT
};

enum class Language {
    EN_US,
    ZH_CN
};

class Localizer {
public:
    static void setLanguage(const std::string& langCode);
    static Language getLanguage();
    static std::string get(ID id);
    static std::string get(const std::string& key);

    static Language parseLanguage(const std::string& langStr);
    static std::string languageToString(Language lang);
    static std::string currentLanguageCode();

private:
    static std::string idToKey(ID id);
    static bool loadFile(const std::string& path);
    static std::string unescapeXML(const std::string& text);

    static std::unordered_map<std::string, std::string> s_translations;
    static std::string s_langCode;
};

}

#endif
