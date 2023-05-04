#pragma once
//  Mite configuration file, suckless style

#define GREEN   "\x1b[1;32m"
#define YELLOW  "\x1b[1;33m"
#define RED     "\x1b[1;31m"

#define BRIGHT_GREEN   "\x1b[38;5;82m"
#define BRIGHT_ORANGE  "\x1b[38;5;208m"
#define BRIGHT_RED     "\x1b[38;5;1m"
#define BRIGHT_BLUE    "\x1b[38;5;81m"
#define RESET   "\x1b[0m"

//
//  Tree-sitter configuration
//

#define tsCpp  tree_sitter_cpp()

#define tsString   BRIGHT_GREEN   //  @string
#define tsKeyword  BRIGHT_ORANGE  //  @keyword
#define tsInclude  BRIGHT_BLUE    //  @include

//  paths to each languages language server
#define cppServer   "clangd"
#define cServer     "clangd"
#define rustServer  "rust-analyzer"
#define javaServer  "/home/coffee/eclipse.jdt.ls/org.eclipse.jdt.ls.product/target/repository/bin/jdtls"

//  Defines wether or not mite shows incomplete completion
//  results from the lsp. False by default
#define LspShowIncomplete  false
