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
    raw.c_iflag &= IGNCR;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void cook() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}


int main(int argc, char *argv[]) {
    ioctl( 0, TIOCGWINSZ, (char *) &size );
    uncook();
    printf("\033[H\033[J");
    char c;
    std::string mode;
    std::string temp;

    cursor Cursor;
    file File;
    screen Screen;
    edit Edit;
    popup Popup;
    Mode = command;
    int x = 1;

    File.filePath = argv[1];
    File.openFile();
    Screen.print(File, Popup, Mode);
    Cursor.move();
    Popup.listIndex = 2;
    std::fflush(stdout);

    while (read(STDIN_FILENO, &c, 1)) {
        if (c == 27) { //  Command mode on ESC
            Mode = command;
        } else if (Mode == insert) {
            Edit.insertMode(File, Screen, Cursor, c);
        } else if (Mode == command) {
            Edit.commandMode(File, Screen, Cursor, c);
        }

        printf("\033[H\033[J");
        Screen.print(File, Popup, Mode);
        Cursor.move();

        if (c == 'c') {
            Popup.list.clear();
        }
        Popup.print(Cursor);

        std::fflush(stdout);
    }
}



