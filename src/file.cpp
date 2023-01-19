#pragma once
#include <string>
#include <vector>
#include <fstream>

struct file {
    std::vector<std::string> vect;
    std::string filePath;

    void openFile() {
        vect.clear();
        std::string line;
        std::ifstream f(filePath);

        while (getline(f, line)) {
            vect.push_back(line);
        }

        f.close();
    } 

    void writeFile() {
        std::ofstream f(filePath);

        for (auto& it : vect) {
            f << it + '\n';
        }
    }
};
