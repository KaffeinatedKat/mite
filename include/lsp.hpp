#pragma once
#define RAPIDJSON_ASSERT(x)
#define err(...) fprintf(stderr, __VA_ARGS__)

#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>
#include <sys/types.h>
#include <poll.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <string>

#include "screen.hpp"
#include "file.hpp"

static std::mutex lock;

struct lsp {
    std::string jsonString, jsonOut, readAction;
    int jOut;
    int version = 1;
    int lspIn[2], lspOut[2], lspErr[2];

    void start(std::string);
    char* readJson();
    void initialize();
    void exit();
    void didOpen(file &, std::string);
    void didChange(file &, screen &, char);
    void completion(file &, screen &, popup &);
    void parseResponse(file &, screen &, cursor &, popup &, winsize, int, char*);
};
