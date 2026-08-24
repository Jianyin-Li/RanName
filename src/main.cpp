#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include "core/config_manager.h"
#include "core/name_list.h"
#include "core/randomizer.h"
#include "ui/console.h"
#include "ui/tui.h"
#include "ui/setup_tui.h"
#include "utils/platform.h"
#include "i18n/localizer.h"

class Application {
private:
    config::ConfigManager configManager;
    data::NameList nameList;
    core::Randomizer* randomizer;
    ui::TUI tui;
    bool langSet;

public:
    Application(bool langSetByArg = false) : randomizer(nullptr), langSet(langSetByArg) {
        initialize();
    }

    ~Application() {
        delete randomizer;
    }

    void initialize() {
        utils::Platform::setUTF8Encoding();
        utils::Platform::createDirectory("data");

        configManager.loadFromFile("data/config.conf");

        if (!langSet) {
            i18n::Localizer::setLanguage(configManager.getLanguage());
        }

        nameList.loadFromFile("data/namelist.txt");

        if (configManager.getNameCount() != nameList.getCount()) {
            configManager.setNameCount(nameList.getCount());
            configManager.saveToFile("data/config.conf");
        }
    }

    void run() {
        if (nameList.isEmpty()) {
            handleEmptyList();
            return;
        }

        setupRandomizer();
        bool hideNext = false;

        while (true) {
            if (!randomizer->hasNext()) {
                handleDone();
                return;
            }

            size_t index = randomizer->getNextIndex();
            const std::string& name = nameList.getNameAt(index);
            size_t current = nameList.getCount() - randomizer->getRemainingCount() - 1;
            size_t total = nameList.getCount();

            std::string nextName;
            if (randomizer->hasNext()) {
                size_t nextIdx = randomizer->peekNextIndex();
                nextName = nameList.getNameAt(nextIdx);
            }

            while (true) {
                tui.showPickScreen(name, current, total,
                                  configManager.getModeDescription(),
                                  nextName, hideNext);

                std::vector<ui::TUIAction> actions = tui.getAction();
                bool advance = false;
                bool restart = false;
                bool setup = false;
                for (ui::TUIAction action : actions) {
                    switch (action) {
                        case ui::TUIAction::NEXT:
                            advance = true;
                            break;
                        case ui::TUIAction::QUIT:
                            return;
                        case ui::TUIAction::HELP:
                            tui.showHelpScreen();
                            break;
                        case ui::TUIAction::ABOUT:
                            tui.showAboutScreen();
                            break;
                        case ui::TUIAction::HIDE:
                            hideNext = !hideNext;
                            break;
                        case ui::TUIAction::MODE_ALL:
                            configManager.setMode(config::PickMode::ALL_RANDOM);
                            restart = true;
                            break;
                        case ui::TUIAction::MODE_ONE:
                            configManager.setMode(config::PickMode::ONE_BY_ONE);
                            restart = true;
                            break;
                        case ui::TUIAction::LANG: {
                            std::string langArg = tui.getLangArg();
                            i18n::Language next;
                            if (!langArg.empty()) {
                                next = i18n::Localizer::parseLanguage(langArg);
                            } else {
                                i18n::Language cur = i18n::Localizer::getLanguage();
                                next = (cur == i18n::Language::ZH_CN)
                                    ? i18n::Language::EN_US : i18n::Language::ZH_CN;
                            }
                            i18n::Localizer::setLanguage(i18n::Localizer::languageToString(next));
                            configManager.setLanguage(i18n::Localizer::languageToString(next));
                            configManager.saveToFile("data/config.conf");
                            utils::Platform::setUTF8Encoding();
                            break;
                        }
                        case ui::TUIAction::RESTART:
                            restart = true;
                            break;
                        case ui::TUIAction::SETUP:
                            setup = true;
                            break;
                        case ui::TUIAction::NONE:
                            break;
                    }
                }
                if (setup) {
                    handleSetup();
                    restart = true;
                }
                if (restart) {
                    setupRandomizer();
                    hideNext = false;
                    break;
                }
                if (advance) break;
            }
        }
    }

private:
    void setupRandomizer() {
        delete randomizer;
        core::RandomMode coreMode;
        switch (configManager.getMode()) {
            case config::PickMode::ONE_BY_ONE:
                coreMode = core::RandomMode::ONE_BY_ONE;
                break;
            case config::PickMode::ALL_RANDOM:
            default:
                coreMode = core::RandomMode::ALL_RANDOM;
                break;
        }
        randomizer = new core::Randomizer(coreMode);
        randomizer->initialize(nameList.getNames());
    }

    void handleDone() {
        tui.showDoneScreen();
        while (true) {
            std::vector<ui::TUIAction> actions = tui.getAction();
            bool restart = false;
            bool setup = false;
            for (ui::TUIAction action : actions) {
                switch (action) {
                    case ui::TUIAction::QUIT:
                        return;
                    case ui::TUIAction::HELP:
                        tui.showHelpScreen();
                        break;
                    case ui::TUIAction::ABOUT:
                        tui.showAboutScreen();
                        break;
                    case ui::TUIAction::RESTART:
                        restart = true;
                        break;
                    case ui::TUIAction::SETUP:
                        setup = true;
                        break;
                    default:
                        break;
                }
            }
            if (setup) {
                handleSetup();
                setupRandomizer();
                run();
                return;
            }
            if (restart) {
                setupRandomizer();
                run();
                return;
            }
            tui.showDoneScreen();
        }
    }

    void handleEmptyList() {
        handleSetup();
        if (!nameList.isEmpty()) {
            run();
        }
    }

    void handleSetup() {
        ui::SetupTUI setupUI;
        bool done = false;
        while (!done) {
            setupUI.showMainScreen(configManager.getModeDescription(), nameList.getCount());
            std::string cmd = setupUI.getCommand();

            std::string lower = cmd;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

            if (lower == "exit" || lower == "quit" || lower == "q" || lower == "back" || lower == "b") {
                done = true;
            } else if (lower == "save") {
                configManager.setNameCount(nameList.getCount());
                configManager.saveToFile("data/config.conf");
                std::ofstream file("data/namelist.txt");
                if (file.is_open()) {
                    for (size_t i = 0; i < nameList.getCount(); i++) {
                        file << nameList.getNameAt(i) << std::endl;
                    }
                    file.close();
                }
                done = true;
            } else if (lower == "list") {
                editList(setupUI);
            }
        }

        nameList.loadFromFile("data/namelist.txt");
        if (configManager.getNameCount() != nameList.getCount()) {
            configManager.setNameCount(nameList.getCount());
            configManager.saveToFile("data/config.conf");
        }
    }

    void editList(ui::SetupTUI& setupUI) {
        bool done = false;
        std::string statusMsg;
        while (!done) {
            std::vector<std::string> names;
            for (size_t i = 0; i < nameList.getCount(); i++) {
                names.push_back(nameList.getNameAt(i));
            }
            setupUI.showListScreen(names, statusMsg);
            std::string cmd = setupUI.getCommand();

            std::string lower = cmd;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

            if (lower == "back" || lower == "b" || lower == "return"
                || lower == "exit" || lower == "q") {
                done = true;
            } else if (lower == "clear") {
                nameList.clear();
                statusMsg = i18n::Localizer::get(i18n::ID::LIST_CLEARED);
            } else if (cmd.size() > 4 && lower.substr(0, 4) == "add ") {
                std::string name = cmd.substr(4);
                if (!name.empty()) {
                    nameList.addName(name);
                    statusMsg.clear();
                } else {
                    statusMsg = i18n::Localizer::get(i18n::ID::INVALID_INPUT);
                }
            } else if (cmd.size() > 5 && lower.rfind("edit ", 0) == 0) {
                statusMsg = doEdit(cmd);
            } else {
                statusMsg = i18n::Localizer::get(i18n::ID::INVALID_INPUT);
            }
        }
    }

    std::string doEdit(const std::string& cmd) {
        // cmd = "edit <id> <name>"
        std::string rest = cmd.substr(5);
        size_t sp = rest.find_first_of(" \t");
        if (sp == std::string::npos || sp == 0) {
            return i18n::Localizer::get(i18n::ID::INVALID_INPUT);
        }
        std::string newName = rest.substr(sp + 1);
        newName.erase(0, newName.find_first_not_of(" \t"));
        newName.erase(newName.find_last_not_of(" \t") + 1);
        if (newName.empty()) {
            return i18n::Localizer::get(i18n::ID::INVALID_INPUT);
        }

        std::string idStr = rest.substr(0, sp);
        size_t idx;
        try {
            idx = std::stoul(idStr);
        } catch (...) {
            return i18n::Localizer::get(i18n::ID::INVALID_INPUT);
        }
        if (idx < 1 || idx > nameList.getCount()) {
            return i18n::Localizer::get(i18n::ID::INVALID_INPUT);
        }
        if (!nameList.setNameAt(idx - 1, newName)) {
            return i18n::Localizer::get(i18n::ID::INVALID_INPUT);
        }
        return i18n::Localizer::get(i18n::ID::LIST_EDITED);
    }
};

int main(int argc, char* argv[]) {
    bool langSetByArg = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--lang" && i + 1 < argc) {
            std::string langStr = argv[++i];
            if (langStr == "zh-CN" || langStr == "zh_CN" || langStr == "zh") {
                i18n::Localizer::setLanguage("zh_CN");
            } else {
                i18n::Localizer::setLanguage("en_US");
            }
            langSetByArg = true;
        }
    }

    Application app(langSetByArg);
    app.run();
    return 0;
}
