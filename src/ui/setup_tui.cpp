#include "setup_tui.h"
#include "../utils/platform.h"
#include "../utils/display_util.h"
#include "../i18n/localizer.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace ui {

static const char* TOP_LEFT     = "\xe2\x95\x94";
static const char* TOP_RIGHT    = "\xe2\x95\x97";
static const char* BOT_LEFT     = "\xe2\x95\x9a";
static const char* BOT_RIGHT    = "\xe2\x95\x9d";
static const char* HORIZ        = "\xe2\x95\x90";
static const char* VERT         = "\xe2\x95\x91";

// ANSI color theme (matches the main TUI and GUI's purple-blue gradient)
static const char* C_RESET  = "\033[0m";
static const char* C_BORDER = "\033[96m";
static const char* C_TITLE  = "\033[1;97m";
static const char* C_MODE   = "\033[33m";
static const char* C_TEXT   = "\033[37m";
static const char* C_PREV   = "\033[2;37m";
static const char* C_STATUS = "\033[93m";
static const char* C_PROMPT = "\033[1;92m";

SetupTUI::SetupTUI()
    : width(78)
    , frameBottomRow(0)
{
    int tw = utils::Platform::getTerminalWidth();
    if (tw > 0) width = tw - 2;
    if (width < 40) width = 40;
    if (width > 120) width = 120;
}

SetupTUI::~SetupTUI() {
    std::cout << "\033[0m\033[?25h" << std::flush;
}

void SetupTUI::clearScreen() {
    std::cout << "\033[2J\033[H";
}

void SetupTUI::drawTopBorder() {
    std::cout << "\033[1;1H" << C_BORDER << TOP_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << TOP_RIGHT << C_RESET;
}

void SetupTUI::drawBottomBorder(int row) {
    std::cout << "\033[" << row << ";1H" << C_BORDER << BOT_LEFT;
    for (int i = 0; i < width; i++) std::cout << HORIZ;
    std::cout << BOT_RIGHT << C_RESET;
}

void SetupTUI::drawContentRow(int row, const std::string& text) {
    std::cout << "\033[" << row << ";1H" << C_BORDER << VERT << C_RESET;
    std::string content = display::fitWidth(text, width);
    std::cout << C_TEXT << content << C_RESET;
    int pad = width - display::displayWidth(content);
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) std::cout << " ";
    std::cout << C_BORDER << VERT << C_RESET;
}

void SetupTUI::drawPrompt() {
    int r = frameBottomRow + 2;
    std::cout << "\033[" << r << ";1H\033[K" << C_PROMPT << "> " << C_RESET << std::flush;
}

void SetupTUI::showMainScreen(const std::string& modeDesc, size_t nameCount,
                               const std::string& statusMsg) {
    clearScreen();
    drawTopBorder();

    std::string title = i18n::Localizer::get(i18n::ID::TITLE_CONFIG);
    int avail = width - 2;
    int pad = avail - display::displayWidth(title) - display::displayWidth(modeDesc) - 6;
    if (pad < 1) pad = 1;
    std::string header = "  " + title + std::string(pad, ' ') + "  " + modeDesc;
    drawContentRow(2, header);

    std::string countStr = i18n::Localizer::get(i18n::ID::NAME_COUNT)
                         + std::to_string(nameCount);
    drawContentRow(3, "  " + countStr);

    drawContentRow(4, "");

    std::string hint1 = i18n::Localizer::get(i18n::ID::SETUP_HINT_LIST);
    drawContentRow(5, "  " + hint1);

    std::string hint2 = i18n::Localizer::get(i18n::ID::SETUP_HINT_SAVE);
    drawContentRow(6, "  " + hint2);

    std::string status = statusMsg.empty()
        ? i18n::Localizer::get(i18n::ID::PRESS_ENTER)
        : statusMsg;
    drawContentRow(7, "  " + status);

    frameBottomRow = 8;
    drawBottomBorder(frameBottomRow);

    std::cout << std::flush;
}

void SetupTUI::showListScreen(const std::vector<std::string>& names,
                               const std::string& statusMsg) {
    clearScreen();
    drawTopBorder();

    std::string title = i18n::Localizer::get(i18n::ID::SETUP_LIST_EDIT);
    drawContentRow(2, "  " + title);

    int row = 3;
    size_t maxDisplay = 7;
    size_t shown = 0;
    for (size_t i = 0; i < names.size() && shown < maxDisplay; i++, shown++) {
        std::string entry = "  " + std::to_string(i + 1) + ". " + names[i];
        drawContentRow(row++, entry);
    }
    if (names.size() > maxDisplay) {
        std::string more = "  ... " + std::to_string(names.size() - maxDisplay)
                         + " " + i18n::Localizer::get(i18n::ID::NAMES_COUNT);
        drawContentRow(row++, more);
    }
    if (names.empty()) {
        std::string empty = "  " + i18n::Localizer::get(i18n::ID::LIST_EMPTY);
        drawContentRow(row++, empty);
    }
    for (int r = row; r <= 6; r++) {
        drawContentRow(r, "");
    }

    std::string hintAdd = i18n::Localizer::get(i18n::ID::SETUP_HINT_ADD);
    drawContentRow(7, "  " + hintAdd);

    std::string hintEdit = i18n::Localizer::get(i18n::ID::SETUP_HINT_EDIT);
    drawContentRow(8, "  " + hintEdit);

    std::string status = statusMsg.empty()
        ? i18n::Localizer::get(i18n::ID::PRESS_ENTER)
        : statusMsg;
    drawContentRow(9, "  " + status);

    frameBottomRow = 10;
    drawBottomBorder(frameBottomRow);

    std::cout << std::flush;
}

std::string SetupTUI::getCommand() {
    drawPrompt();
    std::string line;
    std::getline(std::cin, line);

    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);
    return line;
}

}
