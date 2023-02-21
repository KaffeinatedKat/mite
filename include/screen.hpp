#pragma once
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "file.hpp"
#include "config.hpp"


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
    std::string modes[2] = {"NORMAL", "INSERT"};
    std::string cmdLine;
    int verticalSize, horizontalSize, bottomLine, rightLine;

    int cursorLine = 0, cursorChar = 0, scrollIndex = 0, leftLine = 0;
    
    int topLine = 1;

    void print(file, int);

    void init(winsize);
};
