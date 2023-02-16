#pragma once
#include <cstdio>
#include <vector>
#include <string>

#include "file.hpp"
#include "screen.hpp"

enum mode {
    command = 0,
    insert = 1
};


struct undoInstruction {
    std::string text;
    int index, line, action;
};


struct edit {
    std::string cmdLine2;
    std::vector<undoInstruction> undoStack;
    int undoIndex = -1;

    void insertMode(file &, screen &, cursor &, popup &, char &);

    int commandMode(file &, screen &, cursor &, popup &, mode &, char &);

    void undo(file &, screen &, cursor &);

    void endOfTheLine(file &, screen &, cursor &);
};