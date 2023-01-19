#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <termios.h>

#include "file.cpp"
#include "screen.cpp"
#include "edit.cpp"

struct termios raw;
struct termios orig;



void uncook() {
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void cook() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}


int main(int argc, char *argv[]) {
    uncook();
    printf("\033[H\033[J");
    char c;
    std::string mode;
    std::string temp;

    file File;
    screen Screen;
    edit Edit;
    cursor Cursor;
    Mode = command;
    int x = 1;

    File.filePath = argv[1];
    File.openFile();
    Screen.print(Screen, File, Mode);
    Cursor.move();
    std::fflush(stdout);

    while (read(STDIN_FILENO, &c, 1)) {
        if (c == 27) { //  Command mode on ESC
            Mode = command;
        } else if (Mode == insert) {
            Edit.insertMode(File, Screen, c, Cursor.column - Cursor.offset);
            Cursor.column++;
        } else if (Mode == command) {
            Edit.commandMode(File, Screen, Cursor, c);
        }

        /* else if (c == 'i') {
            Edit.Mode = Edit.insert;
        } else if (c == 'w') {
            File.buildString(File);
        } else if (c == 'r') {
            File.buildVector(File);
        } else if (c == 'l') {
            Screen.scrollIndex += 1;
            Cursor.column += 1;
        } else if (c == 'h') {
            if (Screen.scrollIndex > 0) {
                Screen.scrollIndex -= 1;
                Cursor.column -= 1;
            }
        } else if (c == '$') {
            printf("\n");
        }
        */

        printf("\033[H\033[J");
        Screen.print(Screen, File, Mode);
        printf("\033[%d;%dH", Cursor.row, Cursor.column);
        std::fflush(stdout);
    }
}