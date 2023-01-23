#pragma once
#include <cstdio>
#include <ios>
#include <iostream>

#include "file.cpp"

struct cursor {
    int row = 1;
    int column = 8;
    int offset = 8;

    void move() {
        printf("\033[%d;%dH", row, column);
    }
};

struct screen {
    std::string cmdLine;
    std::string modes[2] = {"command", "insert"};
    int verticalSize = 10;
    int horizontalSize = 20;
    int cursorLine = 0;
    int cursorChar = 0;
    int scrollIndex = 0;
    int topLine = 1;
    int bottomLine = verticalSize;
    int leftLine = 0;
    int rightLine = horizontalSize;

    void print(file File, int m) {
        int line = 0;
        std::string highlight = "";

        for (auto& it : File.vect) {
            line++;

            if (cursorLine > bottomLine) { //  Scrolling down
                bottomLine++;
                topLine++;
            } else if (cursorLine < topLine) { //  Scrolling up
                bottomLine--;
                topLine--;
            }

            if (cursorChar > rightLine) {
                rightLine++;
                leftLine++;
            } else if (cursorChar < leftLine) {
                rightLine--;
                leftLine--;
            }

            if (line < topLine) {
                continue;
            }

            if (line == cursorLine + 1) {
                highlight = "\x1b[40m";
            }
            if (it.length() > rightLine - horizontalSize) {

                printf("%s%-5d| %s\x1b[0m\n", highlight.c_str(), line, it.substr(rightLine - horizontalSize, horizontalSize).c_str());
            } else {
                printf("%s%-5d| \x1b[0m\n", highlight.c_str(), line);
            }
            highlight = "";

            if (line == bottomLine) {
                printf("< %s > %s", modes[m].c_str(), cmdLine.c_str());
                break;
            }
        }
    }
};
