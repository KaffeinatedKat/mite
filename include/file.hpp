#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>


struct error {
    std::string reason, lineText;
    int line, start, end;
};


struct file {
    std::map<int, error> errMap;
    std::vector<std::string> vect;
    std::string filePath, string;

    void openFile();

    void writeFile();

    void toString();
};


