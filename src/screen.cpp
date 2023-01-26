#pragma once
#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "file.cpp"

struct winsize size;

struct cursor {
    int row = 1;
    int column = 8;
    int offset = 8;

    void move() {
        printf("\033[%d;%dH", row, column);
    }
};


struct popup {
    std::vector<std::string> list;
    std::string color = "\x1b[41m";
    int length = 0;
    int listIndex = 0;
    int topLine = 0;
    int bottomLine = 10;

    void print(cursor &Cursor) {
        int line = Cursor.row + 1;
        int index = 0;

        printf("\x1b[B");
        for (auto& it : list) {
            index++;
            if (listIndex == index) { color = "\x1b[42m"; } //  Highlighted item
            printf("%s %s%s\x1b[%d;%dH", color.c_str(), it.c_str(), std::string(length - it.length() + 1, ' ').c_str(),++line, Cursor.column); //  Print item, move back and down, print again
            color = "\x1b[41m"; //  Dehighlight item

            if (index == bottomLine) { break; }
        }
        printf("\x1b[0m");
        Cursor.move(); //  Move cursor back to original position
    }

    void append(std::string value) {
        if (value.length() > length) {
            length = value.length();
        }
        list.push_back(value);
    }

};


struct screen {
    std::string modes[2] = {"command", "insert"};
    std::string cmdLine;
    int verticalSize = size.ws_row - 1;
    int horizontalSize = size.ws_col - 8;
    int cursorLine = 0;
    int cursorChar = 0;
    int scrollIndex = 0;
    int topLine = 1;
    int bottomLine = verticalSize;
    int leftLine = 0;
    int rightLine = horizontalSize;


    void print(file File, popup &Popup, int m) {
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

            if (line > bottomLine) {
                printf("< %s > %s", modes[m].c_str(), cmdLine.c_str());
                break;
            }
        }
    }
};
