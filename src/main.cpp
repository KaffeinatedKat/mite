#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <csignal>
#include <thread>
#include <filesystem>
#include <iostream>

#include "file.hpp"
#include "edit.hpp"
#include "screen.hpp"
#include "lsp.hpp"

#define FDS 2

#ifndef NO_LSP
#else
#undef FDS
#define FDS 1
#endif

struct winsize size;
struct termios raw;
struct termios orig;
struct pollfd pfds[2];


void uncook() {
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_iflag &= ~(ICRNL | IGNBRK);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void cook() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void ctrlc(int) {}


int main(int argc, char *argv[]) {
    ioctl( 0, TIOCGWINSZ, (char *) &size );
    uncook();
    char c;
    std::string mode;
    std::string temp;
    int ready;

    enum::mode Mode;
    cursor Cursor;
    file File;
    screen Screen;
    edit Edit;
    popup Popup;
    lsp Lsp;
    Mode = command;

    signal(SIGINT, ctrlc);
    File.filePath = "/home/coffee/github/mite/" + std::string(argv[1]);
    File.openFile();
    File.toString();
    Screen.print(File, Popup, size, Mode);
    Cursor.move();
    Popup.listIndex = 1;
    std::fflush(stdout);

    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;    
    
#ifndef NO_LSP
    Lsp.start("clangd");
    pfds[1].fd = Lsp.lspOut[0];
    pfds[1].events = POLLIN;
    Lsp.initialize();
    Lsp.didOpen(File, "cpp");
#endif 

    while ((ready = poll(pfds, FDS, -1)) > 0) {
        if (pfds[0].revents & POLLIN) { //  User has typed
            read(STDIN_FILENO, &c, 1);

            if (c == 'q') {
                Lsp.exit();
                cook();
                exit(0);
            }

            if (c == 27) { //  Command mode on ESC
                Mode = command;
                Popup.clr();
            } else if (Mode == insert) {
                File.errMap.clear();
                Edit.insertMode(File, Screen, Cursor, Popup, c);
                Lsp.didChange(File, Screen, c);
                Lsp.completion(File, Screen, Popup);
            } else if (Mode == command) {
                if (Edit.commandMode(File, Screen, Cursor, Popup, Mode, c) == 1) {
                    File.errMap.clear();
                    Lsp.didChange(File, Screen, c);
                }
            }

            Screen.print(File, Popup, size, Mode);
            Cursor.move();
            Popup.print(Cursor);
            std::fflush(stdout);
        }
        if (pfds[1].revents & POLLIN) { //  Language server has responded
            Lsp.parseResponse(File, Screen, Cursor, Popup, size, Mode, Lsp.readJson());
        }
    }
}

