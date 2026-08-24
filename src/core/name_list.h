#ifndef NAME_LIST_H
#define NAME_LIST_H

#include <vector>
#include <string>

namespace data {
    class NameList {
    private:
        std::vector<std::string> names;
        
    public:
        NameList();
        bool loadFromFile(const std::string& filename);
        size_t getCount() const;
        bool isEmpty() const;
        std::string getNameAt(size_t index) const;
        const std::vector<std::string>& getNames() const;
        void addName(const std::string& name);
        bool setNameAt(size_t index, const std::string& name);
        void clear();
    };
}

#endif // NAME_LIST_H
