#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>
#include <ctime>

namespace ui {

enum class TUIAction {
    NEXT,
    QUIT,
    HIDE,
    MODE_ALL,
    MODE_ONE,
    LANG,
    RESTART,
    SETUP,
    HELP,
    ABOUT,
    NONE
};

class TUI {
public:
    TUI();
    ~TUI();

    void showPickScreen(const std::string& name, size_t current, size_t total,
                        const std::string& modeDesc,
                        const std::string& nextName, bool hideNext);
    void showDoneScreen();
    void showEmptyScreen();
    void showHelpScreen();
    void showAboutScreen();

    TUIAction getAction();
    const std::string& getLangArg() const;

private:
    void ansiMove(int row, int col);
    void ansiClearLine();
    void ansiClearScreen();

    void drawFrame();
    void drawHeader(const std::string& modeDesc);
    void drawName(const std::string& name);
    void drawNextPreview(const std::string& nextName, bool hideNext);
    void drawProgress(size_t current, size_t total);
    void drawStatus(const std::string& msg);
    void drawPrompt();
    void drawHelpLine(const std::string& cmd, const std::string& desc, int row);
    void drawInputLine(const std::string& input, size_t cursor);

    std::string readCommand();
    std::string tabComplete(const std::string& input);

    TUIAction parseCommand(const std::string& cmd);
    std::string centerText(const std::string& text, int w);
    std::string fitWidth(const std::string& text, int w);
    int displayWidth(const std::string& text);
    std::string currentTimeStr();

    int width;
    int height;
    std::time_t lastTimeUpdate;
    std::string cachedTimeStr;
    bool drawn;
    std::string lastStatus;
    std::string lastLangArg;
    std::vector<std::string> history;
    int frameBottomRow;
};

}

#endif
