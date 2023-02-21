#pragma once
//  Mite configuration file, suckless style

#define YELLOW  "\x1b[33m"
#define RED     "\x1b[31m"
#define RESET   "\x1b[0m"

//  paths to each languages language server
#define cppServer   "clangd"
#define cServer     "clangd"
#define rustServer  "rust-analyzer"
#define javaServer  "/home/coffee/eclipse.jdt.ls/org.eclipse.jdt.ls.product/target/repository/bin/jdtls"

//  Defines wether or not mite shows incomplete completion
//  results from the lsp. False by default
#define LspShowIncomplete  false
