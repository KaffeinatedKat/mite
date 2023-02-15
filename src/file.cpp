#include "file.hpp"

void file::openFile() {
    vect.clear();
    std::string line;
    std::ifstream f(filePath);

    while (getline(f, line)) {
        vect.push_back(line);
    }

    f.close();
} 

void file::writeFile() {
    std::ofstream f(filePath);

    for (auto& it : vect) {
        f << it + '\n';
    }
}

void file::toString() {
    for (const auto& it : vect) {
        string.append(it + "\r\n");
    }
}
