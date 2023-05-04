#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "file.hpp"
#include "config.hpp"

struct annotations {
    std::string first;
    std::string last;
    std::string errMsg;
    std::string errColor;
    int start;
    int end;
    bool localIndex;
};

struct cursor {
    std::string cursors[2] = {"\033[2 q", "\033[6 q"};
    int row = 1;
    int column = 8;
    int offset = 8;
    void move();
};


struct popup {
    std::vector<std::string> list, text;
    std::vector<int> line, start, end;
    std::string color;
    int length = 0, listIndex = 0, topLine = 0;
    int bottomLine = 10;

    void print(cursor &);
    
    void append(std::string);

    void clr();
};


struct screen {
    std::map<int, std::vector<annotations>> fileAnnotations;
    std::string modes[2] = {"", "-- INSERT -- "};
    std::string cmdLine;
    int verticalSize, horizontalSize, bottomLine, rightLine;

    int cursorLine = 0, cursorChar = 0, scrollIndex = 0, leftLine = 0;
    
    int topLine = 1;

    void print(file, int);

    void init(winsize);

    void clearAnnotations();
};
