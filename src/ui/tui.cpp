#include "tui.h"
#include "../utils/platform.h"
#include "../utils/display_util.h"
#include "../i18n/localizer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

namespace ui {

static const char* TOP_LEFT     = "\xe2\x95\x94";
static const char* TOP_RIGHT    = "\xe2\x95\x97";
static const char* BOT_LEFT     = "\xe2\x95\x9a";
static const char* BOT_RIGHT    = "\xe2\x95\x9d";
static const char* HORIZ        = "\xe2\x95\x90";
static const char* VERT         = "\xe2\x95\x91";
static const char* TOP_MID      = "\xe2\x95\xa6";
static const char* BOT_MID      = "\xe2\x95\xa9";
static const char* LEFT_MID     = "\xe2\x95\xa0";
static const char* RIGHT_MID    = "\xe2\x95\xa3";
static const char* PROGRESS_ON  = "\xe2\x96\x88";
static const char* PROGRESS_OFF = "\xe2\x96\x91";

// ANSI color theme (matches the GUI's purple-blue gradient)
static const char* C_RESET  = "\033[0m";
static const char* C_BORDER = "\033[96m";
static const char* C_TITLE  = "\033[1;97m";
static const char* C_MODE   = "\033[33m";
static const char* C_TIME   = "\033[96m";
static const char* C_NAME   = "\033[1;96m";
static const char* C_PREV   = "\033[2;37m";
static const char* C_PROG   = "\033[92m";
static const char* C_REM    = "\033[93m";
static const char* C_STATUS = "\033[93m";
static const char* C_PROMPT = "\033[1;92m";
static const char* C_HELP_H = "\033[1;96m";
static const char* C_HELP_C = "\033[93m";

namespace {

// Commands offered for Tab completion.
const char* kCommands[] = {
    "next", "quit", "hide", "all", "one",
    "lang", "language", "about", "restart", "setup", "help"
};

// Move the cursor one UTF-8 char to the left.
void cursorLeft(const std::string& s, size_t& cur) {
    if (cur == 0) return;
    --cur;
    while (cur > 0 && (static_cast<unsigned char>(s[cur]) & 0xC0) == 0x80) --cur;
}

// Move the cursor one UTF-8 char to the right.
void cursorRight(const std::string& s, size_t& cur) {
    if (cur >= s.size()) return;
    ++cur;
    while (cur < s.size() && (static_cast<unsigned char>(s[cur]) & 0xC0) == 0x80) ++cur;
}

// Remove the UTF-8 char at position cur.
void removeAt(std::string& s, size_t cur) {
    if (cur >= s.size()) return;
    size_t end = cur + 1;
    while (end < s.size() && (static_cast<unsigned char>(s[end]) & 0xC0) == 0x80) ++end;
    s.erase(cur, end - cur);
}

}  // namespace

TUI::TUI()
    : width(78)
    , height(24)
    , lastTimeUpdate(0)
    , drawn(false)
    , frameBottomRow(8)
{
    int tw = utils::Platform::getTerminalWidth();
    if (tw > 0) width = tw - 2;
    if (width < 40) width = 40;
    if (width > 120) width = 120;
}

TUI::~TUI() {
    utils::Platform::setRawMode(false);
    std::cout << "\033[0m\033[?25h" << std::flush;
}

void TUI::ansiMove(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

void TUI::ansiClearLine() {
    std::cout << "\033[K";
}

void TUI::ansiClearScreen() {
    std::cout << "\033[2J\033[H";
}

// Display width of a UTF-8 string: East Asian wide chars count as 2 columns,
// everything else (incl. block elements like █) as 1 column.
// ANSI escape sequences (\033[...m) are ignored.
int TUI::displayWidth(const std::string& text) {
    return display::displayWidth(text);
}

// Truncate text to fit a display width without breaking a UTF-8 sequence.
// ANSI escape sequences are preserved but not counted toward the width.
std::string TUI::fitWidth(const std::string& text, int w) {
    return display::fitWidth(text, w);
}

std::string TUI::centerText(const std::string& text, int w) {
    return display::centerText(text, w);
}

std::string TUI::currentTimeStr() {
    std::time_t now = std::time(nullptr);
    if (now != lastTimeUpdate) {
        lastTimeUpdate = now;
        char buf[16];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
        cachedTimeStr = buf;
    }
    return cachedTimeStr;
}

void TUI::drawFrame() {
    // Row 0: top border
    ansiMove(1, 1);
    std::cout << C_BORDER << TOP_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << TOP_RIGHT << C_RESET;

    // Content rows 1-4 (header, name area)
    for (int r = 1; r <= 4; r++) {
        ansiMove(r + 1, 1);
        std::cout << C_BORDER << VERT << C_RESET;
        for (int i = 0; i < width; i++) std::cout << " ";
        std::cout << C_BORDER << VERT << C_RESET;
    }

    // Row 5: separator
    ansiMove(6, 1);
    std::cout << C_BORDER << LEFT_MID;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << RIGHT_MID << C_RESET;

    // Rows 6-7: progress, status
    for (int r = 6; r <= 7; r++) {
        ansiMove(r + 1, 1);
        std::cout << C_BORDER << VERT << C_RESET;
        for (int i = 0; i < width; i++) std::cout << " ";
        std::cout << C_BORDER << VERT << C_RESET;
    }

    // Row 8: bottom border
    ansiMove(9, 1);
    std::cout << C_BORDER << BOT_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << BOT_RIGHT << C_RESET;

    frameBottomRow = 8;
}

void TUI::drawHeader(const std::string& modeDesc) {
    std::string title = i18n::Localizer::get(i18n::ID::TITLE_MAIN);
    std::string timeStr = currentTimeStr();

    int avail = width - 2;
    int pad = avail - displayWidth(title) - displayWidth(modeDesc) - displayWidth(timeStr) - 4;
    if (pad < 1) pad = 1;

    ansiMove(2, 2);
    ansiClearLine();
    std::cout << C_TITLE << fitWidth(title, avail) << C_RESET
              << std::string(pad, ' ')
              << C_MODE << fitWidth(modeDesc, avail) << C_RESET
              << "  " << C_TIME << fitWidth(timeStr, avail) << C_RESET;
}

void TUI::drawName(const std::string& name) {
    for (int r = 2; r <= 4; r++) {
        ansiMove(r + 1, 2);
        ansiClearLine();
    }

    std::string centered = centerText(name, width);
    ansiMove(4, 2);
    std::cout << C_NAME << centered << C_RESET;
}

void TUI::drawNextPreview(const std::string& nextName, bool hideNext) {
    std::string label = i18n::Localizer::get(i18n::ID::TUI_NEXT_LABEL);
    std::string display = hideNext ? i18n::Localizer::get(i18n::ID::TUI_HIDDEN) : nextName;
    std::string text = fitWidth(label + " " + display, width - 2);

    ansiMove(6, 2);
    ansiClearLine();
    std::cout << C_PREV << text << C_RESET;
}

void TUI::drawProgress(size_t current, size_t total) {
    int barWidth = width - 30;
    if (barWidth < 10) barWidth = 10;

    int done = static_cast<int>(current) + 1;
    int filled = (total > 0) ? (int)((double)done / total * barWidth) : 0;
    if (filled > barWidth) filled = barWidth;
    if (filled < 0) filled = 0;

    std::string bar;
    bar += C_PROG;
    for (int i = 0; i < filled; i++) bar += PROGRESS_ON;
    for (int i = filled; i < barWidth; i++) bar += PROGRESS_OFF;
    bar += C_RESET;

    int pct = (total > 0) ? (int)((double)done / total * 100.0) : 0;
    if (pct > 100) pct = 100;

    int remaining = (done <= (int)total) ? (int)total - done : 0;
    std::string remLabel = i18n::Localizer::get(i18n::ID::REMAINING_LABEL);
    std::string personUnit = i18n::Localizer::get(i18n::ID::PERSON_UNIT);

    std::ostringstream progressText;
    progressText << "  " << bar << "  " << done << "/" << total
                 << " (" << pct << "%)  "
                 << C_REM << remLabel << remaining << personUnit << C_RESET;

    ansiMove(7, 2);
    ansiClearLine();
    std::cout << fitWidth(progressText.str(), width - 2);
}

void TUI::drawStatus(const std::string& msg) {
    std::string display = fitWidth(msg, width - 2);
    ansiMove(8, 2);
    ansiClearLine();
    if (!display.empty()) std::cout << C_STATUS << display << C_RESET;
}

void TUI::showPickScreen(const std::string& name, size_t current, size_t total,
                          const std::string& modeDesc,
                          const std::string& nextName, bool hideNext) {
    if (!drawn) {
        ansiClearScreen();
        drawFrame();
        drawn = true;
    }

    drawHeader(modeDesc);
    drawName(name);
    drawNextPreview(nextName, hideNext);
    drawProgress(current, total);
    drawStatus(lastStatus);
    lastStatus.clear();

    std::cout << std::flush;
}

void TUI::showDoneScreen() {
    drawn = false;
    ansiClearScreen();

    std::string msg = i18n::Localizer::get(i18n::ID::TUI_DONE);
    std::string hint = i18n::Localizer::get(i18n::ID::TUI_HINT_DONE);

    std::cout << C_BORDER << "\033[1;1H" << TOP_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << TOP_RIGHT << C_RESET;
    for (int r = 1; r <= 4; r++) {
        std::cout << "\033[" << (r + 1) << ";1H" << C_BORDER << VERT << C_RESET;
        for (int i = 0; i < width; i++) std::cout << " ";
        std::cout << C_BORDER << VERT << C_RESET;
    }
    std::cout << "\033[3;2H" << C_NAME << centerText(msg, width) << C_RESET;
    std::cout << "\033[5;2H" << C_PREV << centerText(hint, width) << C_RESET;
    std::cout << C_BORDER << "\033[6;1H" << BOT_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << BOT_RIGHT << C_RESET;

    frameBottomRow = 5;
    std::cout << std::flush;
}

void TUI::showEmptyScreen() {
    drawn = false;
    ansiClearScreen();

    std::string msg = i18n::Localizer::get(i18n::ID::TUI_EMPTY);
    std::string hint = i18n::Localizer::get(i18n::ID::TUI_HINT_EXIT);

    std::cout << C_BORDER << "\033[1;1H" << TOP_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << TOP_RIGHT << C_RESET;
    for (int r = 1; r <= 4; r++) {
        std::cout << "\033[" << (r + 1) << ";1H" << C_BORDER << VERT << C_RESET;
        for (int i = 0; i < width; i++) std::cout << " ";
        std::cout << C_BORDER << VERT << C_RESET;
    }
    std::cout << "\033[3;2H" << C_NAME << centerText(msg, width) << C_RESET;
    std::cout << "\033[5;2H" << C_PREV << centerText(hint, width) << C_RESET;
    std::cout << C_BORDER << "\033[6;1H" << BOT_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << BOT_RIGHT << C_RESET;

    frameBottomRow = 5;
    std::cout << std::flush;
}

void TUI::showHelpScreen() {
    drawn = false;
    ansiClearScreen();

    std::string title = i18n::Localizer::get(i18n::ID::TUI_HELP_TITLE);

    std::cout << C_BORDER << "\033[1;1H" << TOP_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << TOP_RIGHT << C_RESET;

    std::cout << "\033[2;1H" << C_BORDER << VERT << C_RESET
              << C_TITLE << centerText(title, width) << C_RESET
              << C_BORDER << VERT << C_RESET;

    std::cout << C_BORDER << "\033[3;1H" << LEFT_MID;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << RIGHT_MID << C_RESET;

    drawHelpLine("next / n", i18n::Localizer::get(i18n::ID::TUI_HELP_NEXT), 4);
    drawHelpLine("hide / h", i18n::Localizer::get(i18n::ID::TUI_HELP_HIDE), 5);
    drawHelpLine("all / one", i18n::Localizer::get(i18n::ID::TUI_HELP_MODE), 6);
    drawHelpLine("lang / language [code]", i18n::Localizer::get(i18n::ID::TUI_HELP_LANG), 7);
    drawHelpLine("restart / r", i18n::Localizer::get(i18n::ID::TUI_HELP_RESTART), 8);
    drawHelpLine("setup / s", i18n::Localizer::get(i18n::ID::TUI_HELP_SETUP), 9);
    drawHelpLine("quit / q", i18n::Localizer::get(i18n::ID::TUI_HELP_QUIT), 10);
    drawHelpLine("help / ?", i18n::Localizer::get(i18n::ID::TUI_HELP_HELP), 11);
    drawHelpLine("! <cmd>", i18n::Localizer::get(i18n::ID::TUI_HELP_SHELL), 12);
    drawHelpLine("about / a", i18n::Localizer::get(i18n::ID::TUI_HELP_ABOUT), 13);

    std::cout << C_BORDER << "\033[14;1H" << BOT_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << BOT_RIGHT << C_RESET;

    frameBottomRow = 14;

    // Wait for the user to dismiss, otherwise the pick screen redraws over it
    int r = frameBottomRow + 2;
    ansiMove(r + 1, 1);
    ansiClearLine();
    std::string backHint = i18n::Localizer::get(i18n::ID::TUI_HELP_BACK);
    std::cout << C_PREV << fitWidth(backHint, width - 2) << C_RESET << std::flush;

    std::string line;
    std::getline(std::cin, line);

    ansiClearScreen();
}

void TUI::showAboutScreen() {
    drawn = false;
    ansiClearScreen();

    std::string title = i18n::Localizer::get(i18n::ID::ABOUT_TITLE);
    std::string project = i18n::Localizer::get(i18n::ID::ABOUT_PROJECT) + APP_VERSION;
    std::string desc = i18n::Localizer::get(i18n::ID::ABOUT_DESC);
    std::string author = i18n::Localizer::get(i18n::ID::ABOUT_AUTHOR);

    std::cout << C_BORDER << "\033[1;1H" << TOP_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << TOP_RIGHT << C_RESET;

    std::cout << "\033[2;1H" << C_BORDER << VERT << C_RESET
              << C_TITLE << centerText(title, width) << C_RESET
              << C_BORDER << VERT << C_RESET;

    std::cout << C_BORDER << "\033[3;1H" << LEFT_MID;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << RIGHT_MID << C_RESET;

    auto row = [&](int r, const std::string& text) {
        std::cout << "\033[" << r << ";1H" << C_BORDER << VERT << C_RESET
                  << C_PREV << centerText(text, width) << C_RESET
                  << C_BORDER << VERT << C_RESET;
    };
    row(4, project);
    row(5, desc);
    row(6, author);

    std::cout << C_BORDER << "\033[7;1H" << BOT_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << BOT_RIGHT << C_RESET;

    frameBottomRow = 7;

    int r = frameBottomRow + 2;
    ansiMove(r + 1, 1);
    ansiClearLine();
    std::string backHint = i18n::Localizer::get(i18n::ID::TUI_HELP_BACK);
    std::cout << C_PREV << fitWidth(backHint, width - 2) << C_RESET << std::flush;

    std::string line;
    std::getline(std::cin, line);

    ansiClearScreen();
}

void TUI::drawHelpLine(const std::string& cmd, const std::string& desc, int row) {
    std::string line = "  " + cmd;
    int pad = width - 2 - displayWidth(line) - displayWidth(desc) - 2;
    if (pad < 2) pad = 2;
    line += std::string(pad, ' ') + desc;

    std::cout << "\033[" << row << ";1H" << C_BORDER << VERT << C_RESET
              << C_HELP_C << fitWidth(line, width) << C_RESET
              << C_BORDER << VERT << C_RESET;
}

void TUI::drawPrompt() {
    int r = frameBottomRow + 2;
    ansiMove(r + 1, 1);
    ansiClearLine();
    std::cout << C_PROMPT << "> " << C_RESET << std::flush;
}

const std::string& TUI::getLangArg() const {
    return lastLangArg;
}

void TUI::drawInputLine(const std::string& input, size_t cursor) {
    int row = frameBottomRow + 2;
    ansiMove(row + 1, 1);
    ansiClearLine();
    std::cout << C_PROMPT << "> " << C_RESET << input << std::flush;
    int col = 3 + display::displayWidth(input.substr(0, cursor));
    ansiMove(row + 1, col);
}

std::string TUI::readCommand() {
    utils::Platform::setRawMode(true);
    std::string input;
    size_t cursor = 0;
    int histIndex = static_cast<int>(history.size());
    drawInputLine(input, cursor);

    while (true) {
        unsigned char c = static_cast<unsigned char>(utils::Platform::getChar());
#ifdef _WIN32
        if (c == 0xE0 || c == 0x00) {  // Windows extended key prefix
            int k = utils::Platform::getChar();
            if (k == 0x48) {            // Up
                if (histIndex > 0) {
                    --histIndex;
                    input = history[histIndex];
                    cursor = input.size();
                    drawInputLine(input, cursor);
                }
            } else if (k == 0x50) {     // Down
                if (histIndex < static_cast<int>(history.size())) {
                    ++histIndex;
                    input = (histIndex == static_cast<int>(history.size())) ? "" : history[histIndex];
                    cursor = input.size();
                    drawInputLine(input, cursor);
                }
            } else if (k == 0x4B) {     // Left
                cursorLeft(input, cursor);
                drawInputLine(input, cursor);
            } else if (k == 0x4D) {     // Right
                cursorRight(input, cursor);
                drawInputLine(input, cursor);
            } else if (k == 0x47) {     // Home
                cursor = 0;
                drawInputLine(input, cursor);
            } else if (k == 0x4F) {     // End
                cursor = input.size();
                drawInputLine(input, cursor);
            } else if (k == 0x53) {     // Delete
                removeAt(input, cursor);
                drawInputLine(input, cursor);
            }
            continue;
        }
#else
        if (c == 0x1B) {  // ANSI escape sequence (POSIX arrow keys)
            char s1 = utils::Platform::getChar();
            if (s1 == '[') {
                char s2 = utils::Platform::getChar();
                if (s2 == 'A') {         // Up
                    if (histIndex > 0) {
                        --histIndex;
                        input = history[histIndex];
                        cursor = input.size();
                        drawInputLine(input, cursor);
                    }
                } else if (s2 == 'B') {  // Down
                    if (histIndex < static_cast<int>(history.size())) {
                        ++histIndex;
                        input = (histIndex == static_cast<int>(history.size())) ? "" : history[histIndex];
                        cursor = input.size();
                        drawInputLine(input, cursor);
                    }
                } else if (s2 == 'C') {  // Right
                    cursorRight(input, cursor);
                    drawInputLine(input, cursor);
                } else if (s2 == 'D') {  // Left
                    cursorLeft(input, cursor);
                    drawInputLine(input, cursor);
                } else if (s2 == 'H') {  // Home
                    cursor = 0;
                    drawInputLine(input, cursor);
                } else if (s2 == 'F') {  // End
                    cursor = input.size();
                    drawInputLine(input, cursor);
                } else if (s2 == '1' || s2 == '3' || s2 == '4') {
                    char s3 = utils::Platform::getChar();
                    if (s2 == '1' && s3 == '~') {          // Home
                        cursor = 0;
                        drawInputLine(input, cursor);
                    } else if (s2 == '4' && s3 == '~') {   // End
                        cursor = input.size();
                        drawInputLine(input, cursor);
                    } else if (s2 == '3' && s3 == '~') {   // Delete
                        removeAt(input, cursor);
                        drawInputLine(input, cursor);
                    }
                }
            }
            continue;
        }
#endif
        if (c == '\r' || c == '\n') {
            std::cout << std::endl;
            utils::Platform::setRawMode(false);
            if (!input.empty() && (history.empty() || history.back() != input)) {
                history.push_back(input);
            }
            return input;
        } else if (c == '\t') {
            std::string completed = tabComplete(input);
            if (completed != input) {
                input = completed;
                cursor = input.size();
                drawInputLine(input, cursor);
            }
        } else if (c == '\b' || c == 0x7f) {  // Backspace
            if (cursor > 0) {
                cursorLeft(input, cursor);
                removeAt(input, cursor);
                drawInputLine(input, cursor);
            }
        } else if (c == 0x03) {  // Ctrl+C
            std::cout << std::endl;
            utils::Platform::setRawMode(false);
            return std::string();
        } else if (c >= 0x20) {
            input.insert(cursor, 1, c);
            ++cursor;
            drawInputLine(input, cursor);
        }
    }
}

std::string TUI::tabComplete(const std::string& input) {
    if (input.empty()) return input;
    std::string best;
    for (const char* cmd : kCommands) {
        std::string c(cmd);
        if (c.compare(0, input.size(), input) == 0) {
            if (best.empty()) {
                best = c;
            } else {
                size_t i = 0;
                while (i < best.size() && i < c.size() && best[i] == c[i]) ++i;
                best = best.substr(0, i);
            }
        }
    }
    return best.empty() ? input : best;
}

TUIAction TUI::parseCommand(const std::string& cmd) {
    std::string input = cmd;
    input.erase(0, input.find_first_not_of(" \t"));
    input.erase(input.find_last_not_of(" \t") + 1);
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    if (input.empty()) return TUIAction::NONE;

    if (input == "next" || input == "n") return TUIAction::NEXT;
    if (input == "quit" || input == "q" || input == "exit") return TUIAction::QUIT;
    if (input == "hide" || input == "h") return TUIAction::HIDE;
    if (input == "all" || input == "mode all" || input == "mode_all") return TUIAction::MODE_ALL;
    if (input == "one" || input == "mode one" || input == "mode_one" || input == "mode one by one") return TUIAction::MODE_ONE;
    if (input == "lang" || input == "language" || input == "l") {
        lastLangArg.clear();
        return TUIAction::LANG;
    }
    if (input.rfind("lang ", 0) == 0 || input.rfind("language ", 0) == 0
        || input.rfind("l ", 0) == 0) {
        size_t sp = input.find(' ');
        lastLangArg = input.substr(sp + 1);
        lastLangArg.erase(0, lastLangArg.find_first_not_of(" \t"));
        lastLangArg.erase(lastLangArg.find_last_not_of(" \t") + 1);
        return TUIAction::LANG;
    }
    if (input == "restart" || input == "r") return TUIAction::RESTART;
    if (input == "setup" || input == "config" || input == "settings" || input == "s") return TUIAction::SETUP;
    if (input == "help" || input == "?") return TUIAction::HELP;
    if (input == "about" || input == "a") return TUIAction::ABOUT;

    lastStatus = "Unknown: " + cmd + ". Type 'help' for commands.";
    return TUIAction::NONE;
}

TUIAction TUI::getAction() {
    std::string line = readCommand();
    if (!line.empty() && line[0] == '!') {
        // Run an external shell command (e.g. !dir, !cls)
        ansiClearScreen();
        std::string cmd = line.substr(1);
        std::cout << C_TITLE << "$ " << cmd << C_RESET << std::endl;
        int rc = std::system(cmd.c_str());
        std::cout << C_STATUS << "[exit " << rc << "]" << C_RESET << std::endl;
        utils::Platform::sleep(400);
        drawn = false;
        return TUIAction::NONE;
    }
    return parseCommand(line);
}

}
