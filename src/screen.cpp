#pragma once
#include <cstdio>
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
    int cursorPos = 1;
    int scrollIndex = 0;
    int topLine = 1;
    int bottomLine = verticalSize;

    void print(screen &self, file File, int m) {
        int line = 0;
        int linesToPrint = self.verticalSize - 1;
        std::string highlight = "";

        for (auto& it : File.vect) {
            line++;

            if (cursorPos > bottomLine) { //  Scrolling down
                bottomLine++;
                topLine++;
            } else if (cursorPos < topLine) { //  Scrolling up
                bottomLine--;
                topLine--;
            }

            if (line < topLine) {
                continue;
            }


            if (line == self.cursorPos) {
                highlight = "\x1b[40m";
            }
            if (it.length() > scrollIndex) {

                printf("%s%-5d| %s\x1b[0m\n", highlight.c_str(), line, it.substr(scrollIndex, horizontalSize).c_str());
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
