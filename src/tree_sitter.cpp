#include "tree_sitter.hpp"

#ifndef NO_TS
#include <tree_sitter/api.h>

extern "C" {
    TSLanguage *tree_sitter_cpp();
}


void tree_sitter::language() {
    parser = ts_parser_new();

    ts_parser_set_language(parser, tree_sitter_cpp());
}


void tree_sitter::buildTree(file &File) {
    tree = ts_parser_parse_string(
        parser,
        NULL,
        File.string.c_str(),
        strlen(File.string.c_str())
    );

    root_node = ts_tree_root_node(tree);

    printf("%s\n\n", ts_node_string(root_node));
}

void tree_sitter::highlight(file &File, screen &Screen) {
    std::string kiwi = "\"#include\" @include\n\n[\"default\"\"enum\"\"struct\"\"typedef\"\"union\"\"goto\"] @keyword\n\n\"return\" @keyword.return\n\n[(system_lib_string)\n(string_literal)\n] @string\n\n(ERROR) @error\n\n(true) @boolean\n\n(type_qualifier) @type.qualifier";
    std::string color;
    int line = 0;
    uint32_t *errorOffset = nullptr;

    annotations Highlights;

    query = ts_query_new(tsCpp, kiwi.c_str(), strlen(kiwi.c_str()), errorOffset, &error);
    query_cursor = ts_query_cursor_new();
    ts_query_cursor_exec(query_cursor, query, root_node);
    
    while (ts_query_cursor_next_match(query_cursor, &match)) {
        std::string color;
        std::string pattern = ts_query_capture_name_for_id(query, match.pattern_index, &match.id);
        const TSQueryCapture *capture = match.captures;
        TSNode match_node = capture->node;

        if (pattern == "string") {
            color = tsString;
        } else if (pattern == "keyword") {
            color = tsKeyword;
        } else if (pattern == "keyword.return") {
            color = RED;
        } else if (pattern == "include") {
            color = tsInclude;
        } else {
            continue;
        }

        Highlights.localIndex = false;
        Highlights.first = color;
        Highlights.last = RESET;
        Highlights.start = (int) ts_node_start_byte(match_node);
        Highlights.end = (int) ts_node_end_byte(match_node);


        while (true) {
            if (Highlights.start < File.vectIndicies[line + 1]) {
                Screen.fileAnnotations[line + 1].push_back(Highlights);
                break;
            } else {
                line++;
            }
        }
    }
}
#else

void tree_sitter::highlight(file &, screen &) {}
void tree_sitter::buildTree(file &) {}
void tree_sitter::language() {}

#endif
