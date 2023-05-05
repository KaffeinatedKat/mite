#pragma once
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>

#include "config.hpp"


struct error {
    std::string reason, lineText;
    int line, start, end;
};


struct file {
    std::map<int, error> errMap;
    std::vector<std::string> vect;
    std::vector<int> vectIndicies;
    std::string filePath, string, languageId;
    std::string languageServer = "NONE";

    std::string openFile(std::string);

    void writeFile();

    void toString();

    void getLangId();
};


