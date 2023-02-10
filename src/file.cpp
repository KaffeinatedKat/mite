#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>

struct error {
    std::string reason;
    std::string lineText;
    int line;
    int start;
    int end;
};

struct file {
    std::map<int, error> errMap;
    std::vector<std::string> vect;
    std::string filePath;
    std::string string;

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

    void toString() {
        for (const auto& it : vect) {
            string.append(it + "\r\n");
        }
    }
};
