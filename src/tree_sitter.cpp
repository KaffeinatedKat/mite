#include "tree_sitter.hpp"
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
/*
int main() {
    tree_sitter TS;
    std::string sause = "#include <string>\n#include <deez>\n#include <fart>\n\nstruct deez {};\n\nint main() {\n  int dog = 69;\n   return \"sdfsdf\";\n}";
    std::string kiwi = "\"#include\" @include\n\n[\"default\"\"enum\"\"struct\"\"typedef\"\"union\"\"goto\"] @keyword\n\n\"return\" @keyword.return\n\n[(system_lib_string)\n(string_literal)\n] @string\n\n(ERROR) @error\n\n(true) @boolean\n\n(type_qualifier) @type.qualifier";
    std::string output;
    int offset = 0;
    std::vector<uint32_t> hStart, hEnd;
    std::vector<std::string> hColor;

    TS.language();
    TS.buildTree(sause);


    uint32_t *eOffset = nullptr;
    TS.ts_query = ts_query_new(tree_sitter_cpp(), kiwi.c_str(), strlen(kiwi.c_str()), eOffset, TS.error);
    TS.q_cursor = ts_query_cursor_new();
    ts_query_cursor_exec(TS.q_cursor, TS.ts_query, TS.root_node);
    printf("%s\n\n%s\n\n", kiwi.c_str(), sause.c_str());
    while (ts_query_cursor_next_match(TS.q_cursor, &TS.match)) {
        std::string color;
        std::string reset = "\x1b[0m";
        std::string pattern = ts_query_capture_name_for_id(TS.ts_query, TS.match.pattern_index, &TS.match.id);
        const TSQueryCapture *capture = TS.match.captures;
        TSNode nody = capture->node;

        //printf("fortnut at index: %d | The fucker is %s\n", TS.match.id, ts_query_capture_name_for_id(TS.ts_query, TS.match.pattern_index, &TS.match.id));
        //printf("WE GOTTEM: %s\n", ts_node_string(nody));

        if (pattern == "include") {
            color = "\x1b[38;5;37m";
        } else if (pattern == "string") {
            color = "\x1b[38;5;46m";
        } else if (pattern == "keyword.return") {
            color = "\x1b[38;5;160m";
        } else if (pattern == "keyword") {
            color = "\x1b[38;5;214m";
        }

        //printf("%.*s\x1b[0m", (int)ts_node_end_byte(nody) - ts_node_start_byte(nody),
        //   sause.c_str() + ts_node_start_byte(nody));
        //printf("Start Index: %d\nEnd Index: %d\n\n", ts_node_start_byte(nody), ts_node_end_byte(nody));
        sause.insert((int)ts_node_start_byte(nody) + offset, color);
        offset += color.size();
        sause.insert((int)ts_node_end_byte(nody) + offset, reset);
        offset += reset.size();
    }

    printf("%s\n", sause.c_str());
}*/
