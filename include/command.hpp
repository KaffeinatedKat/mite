#pragma once
#include <string>

#include <unistd.h>
#include <termios.h>

#include "screen.hpp"
#include "lsp.hpp"

void quit(lsp &);

struct command {
    int get(screen &, cursor &, std::string &);
    void execute(file &, screen &, lsp &, std::string);
};
