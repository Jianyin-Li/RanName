#include "name_list.h"
#include "../i18n/localizer.h"
#include <fstream>
#include <iostream>

namespace data {
    NameList::NameList() {}

    bool NameList::loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << i18n::Localizer::get(i18n::ID::UNABLE_OPEN_LIST) << filename << std::endl;
            return false;
        }

        names.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                names.push_back(line);
            }
        }
        file.close();
        return true;
    }

    size_t NameList::getCount() const {
        return names.size();
    }

    bool NameList::isEmpty() const {
        return names.empty();
    }

    std::string NameList::getNameAt(size_t index) const {
        if (index < names.size()) {
            return names[index];
        }
        return "";
    }

    const std::vector<std::string>& NameList::getNames() const {
        return names;
    }

    void NameList::addName(const std::string& name) {
        names.push_back(name);
    }

    bool NameList::setNameAt(size_t index, const std::string& name) {
        if (index >= names.size()) return false;
        names[index] = name;
        return true;
    }

    void NameList::clear() {
        names.clear();
    }
}
