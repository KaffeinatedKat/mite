#pragma once
#include <cstdio>
#include <vector>
#include <string>

#include "file.hpp"
#include "screen.hpp"
#include "command.hpp"

enum mode {
    command = 0,
    insert = 1
};


struct undoInstruction {
    std::string text;
    int index, line, cursorLine, action;
    int topLine, bottomLine;
};


struct edit {
    std::string cmdLine2;
    std::vector<undoInstruction> undoStack;
    int undoIndex = -1;

    int insertMode(file &, screen &, cursor &, popup &, char &);

    int commandMode(file &, screen &, cursor &, lsp &, struct::command &, mode &, char &);

    void undo(file &, screen &, cursor &);

    void endOfTheLine(file &, screen &, cursor &);
};
