#include <string>
#include <cstring>
#include <tree_sitter/api.h>
#include <tree_sitter/parser.h>

#include "file.hpp"
#include "screen.hpp"
#include "config.hpp"


struct tree_sitter {
    TSParser *parser;
    TSTree *tree;
    TSNode root_node;
    TSQuery *query;
    TSQueryCursor *query_cursor;
    TSTreeCursor cursor;
    TSQueryMatch match;
    TSQueryError error;

    static const char *ts_input_read(void *, uint32_t, TSPoint, uint32_t *);

    void language();
    void buildTree(file &);
    void highlight(file &, screen &);
};
