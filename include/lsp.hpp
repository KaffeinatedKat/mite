#pragma once
#define RAPIDJSON_ASSERT(x)
#define err(...) fprintf(stderr, __VA_ARGS__)

#include <cstdlib>
#include <filesystem>
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

//  Language server paths per language
#define cppServer   "clangd"
#define cServer     "clangd"

static std::mutex lock;

struct lsp {
    std::string jsonString, jsonOut, readAction;
    int jOut;
    int version = 1;
    int lspIn[2], lspOut[2], lspErr[2];
    bool running = false;

    void start(file);
    char* readJson();
    void initialize();
    void exit();
    void didOpen(file &);
    void didChange(file &, screen &, char);
    void completion(file &, screen &, popup &);
    void parseResponse(file &, screen &, cursor &, popup &, int, char*);
};
