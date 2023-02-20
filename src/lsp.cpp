#include "lsp.hpp"

#ifndef NO_LSP

using namespace rapidjson;


void lsp::start(file File) {
    if (File.languageServer == "NONE") {
        return;
    }
    running = true;

    //  Pipe stdin, stdout, and stderr from the language server
    if (pipe(lspIn) == -1) { err("pipe 1 failed"); }
    if (pipe(lspOut) == -1) { err("pipe 1 failed"); }
    if (pipe(lspErr) == -1) { err("pipe 1 failed"); }

    int pid = fork();

    if (pid == 0) {
        dup2(lspIn[0], 0);
        dup2(lspOut[1], 1);
        //dup2(lspErr[1], 2);
        char *args[] = {(char *) File.languageServer.c_str(), NULL };
        execvp(File.languageServer.c_str(), args);
    }
}


char* lsp::readJson() {
    if (running == false) { 
        char* retVal = (char *) "";
        return retVal;
    }

    struct pollfd fd;
    char buf[2048] = {0};
    char *json = {0};
    int jsonSize = 0;

    fd.fd = lspOut[0];
    fd.events = POLLIN;

    lock.lock();
    read(lspOut[0], &buf, 16); //  Read 'Content-Length: ' out of the buffer

    //  Then read each byte and store the number till a carage return
    //  that is the length of the json reponse from the server
    while (true) { 
        read(lspOut[0], &buf, 1);
        if (*buf == '\r') {
            jsonSize /= 10;
            read(lspOut[0], &buf, 3); //  Read the remaining '\n\r\n' out of the buffer
            break;
        }
        jsonSize += (*buf - '0');
        jsonSize *= 10;
    }

    json = new char[jsonSize];
    read(lspOut[0], json, jsonSize); //  Read the json response 
    lock.unlock();
    return json;
}



//  FIXME: This json object should not be built, does not change and should be a string
//  like the rest of the functions

void lsp::initialize() {
    if (running == false) {return; }

    std::string initMsg;
    std::string initNotif;
    Document initializeMsg;
    Document initNotification;
    initializeMsg.SetObject();
    initNotification.SetObject();

    Document::AllocatorType& allocator = initializeMsg.GetAllocator();
    Document::AllocatorType& allocator2 = initNotification.GetAllocator();

    initializeMsg.AddMember("jsonrpc", "2.0", allocator);
    initializeMsg.AddMember("id", 1, allocator);
    initializeMsg.AddMember("method", "initialize", allocator);
    initializeMsg.AddMember("params", {}, allocator);
    initializeMsg["params"].SetObject();
    initializeMsg["params"].AddMember("capabilities", {}, allocator);

    initializeMsg["params"]["capabilities"].SetObject();

    initNotification.AddMember("jsonrpc", "2.0", allocator2);
    initNotification.AddMember("method", "initialized", allocator2);
    initNotification.AddMember("params", {}, allocator2);
    initNotification["params"].SetObject();

    StringBuffer sb;
    StringBuffer sb2;
    PrettyWriter<StringBuffer> writer(sb);
    PrettyWriter<StringBuffer> writer2(sb2);
    initializeMsg.Accept(writer);
    initNotification.Accept(writer2);

    initMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
    initNotif = "Content-Length: " + std::to_string(std::string(sb2.GetString()).length()) + "\r\n\r\n" + sb2.GetString();
    write(lspIn[1], initMsg.c_str(), initMsg.length());
    
    Document res;
    res.Parse(readJson());
    StringBuffer sb3;
    PrettyWriter<StringBuffer> writer3(sb3);
    res.Accept(writer3);
    //printf("\n\n%s\n\n", sb3.GetString());

    write(lspIn[1], initNotif.c_str(), initNotif.length());
}


void lsp::exit() {
    if (running == false) { return; }

    const char* json = "{\"jsonrpc\": \"2.0\",\"method\": \"exit\",\"params\": {}}";
    std::string exitMsg;

    Document exit;
    exit.Parse(json);
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    exit.Accept(writer);

    exitMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
    write(lspIn[1], exitMsg.c_str(), exitMsg.length());
}


void lsp::didOpen(file &File) {
    if (running == false) { return; }

    std::string openMsg;
    const char* json = "{\"jsonrpc\": \"2.0\",\"method\": \"textDocument/didOpen\",\"params\":{\"textDocument\": {\"uri\": \"\",\"languageId\": \"\",\"version\": 1,\"text\": \"\"}}}";
    std::string uri = "file://" + File.filePath;

    lock.lock();
    
    Document didOpen;
    didOpen.Parse(json);
    Document::AllocatorType& allocator = didOpen.GetAllocator();

    didOpen["params"]["textDocument"]["uri"].SetString(uri.c_str(), allocator);
    didOpen["params"]["textDocument"]["languageId"].SetString(File.languageId.c_str(), allocator);
    didOpen["params"]["textDocument"]["text"].SetString(File.string.c_str(), allocator);

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    didOpen.Accept(writer);

    //printf("\n\n%s\n\n", sb.GetString());

    openMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
    write(lspIn[1], openMsg.c_str(), openMsg.length());
    //readJson();
    lock.unlock();
}


void lsp::didChange(file &File, screen &Screen, char c) {
    if (running == false) { return; }

    const char* json = "{\"jsonrpc\": \"2.0\",\"method\": \"textDocument/didChange\", \"params\": {\"textDocument\": {\"uri\": \"\",\"version\": \"\"},\"contentChanges\": [{\"range\": {\"start\": {\"line\": 0, \"character\": 0}, \"end\": {\"line\": 0, \"character\": 0}},\"text\": \"\"}, {\"range\": {\"start\": {\"line\": 0, \"character\": 0}, \"end\": {\"line\": 0, \"character\": 0}},\"text\": \"\"}]}}";
    std::string didChangeMsg;
    std::string uri = "file://" + File.filePath;
    std::string text = File.vect[Screen.cursorLine];
    int line = Screen.cursorLine;
    text.append("\r\n");


    lock.lock();

    Document didChange;
    didChange.Parse(json);
    Document::AllocatorType& allocator = didChange.GetAllocator();

    didChange["params"]["textDocument"]["uri"].SetString(uri.c_str(), allocator);
    didChange["params"]["textDocument"]["version"].SetInt(++version);

    if (c == 13) { //  Newline insert 
        line--;
        //  When a newline happens, update lsp on what the current line and
        //  the previous line are.
        text = File.vect[Screen.cursorLine - 1];
        text.append("\r\n" + File.vect[Screen.cursorLine] + "\r\n");
    }

    //  FIXME: this shouldn't have to be 2 edit commands

    //  Replace the current line with an empty string
    didChange["params"]["contentChanges"][0]["range"]["start"]["line"].SetInt(line);
    didChange["params"]["contentChanges"][0]["range"]["start"]["character"].SetInt(0);
    didChange["params"]["contentChanges"][0]["range"]["end"]["line"].SetInt(line + 1);
    didChange["params"]["contentChanges"][0]["range"]["end"]["character"].SetInt(0);
    didChange["params"]["contentChanges"][0]["text"].SetString("", allocator);

    //  Then set that line to the new text from the editor
    didChange["params"]["contentChanges"][1]["range"]["start"]["line"].SetInt(line);
    didChange["params"]["contentChanges"][1]["range"]["start"]["character"].SetInt(0);
    didChange["params"]["contentChanges"][1]["range"]["end"]["line"].SetInt(line);
    didChange["params"]["contentChanges"][1]["range"]["end"]["character"].SetInt(0);
    didChange["params"]["contentChanges"][1]["text"].SetString(text.c_str(), allocator);


    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    didChange.Accept(writer);

    printf("\n\n%s\n\n", sb.GetString());

    didChangeMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
    write(lspIn[1], didChangeMsg.c_str(), didChangeMsg.length());
    //readJson();
    lock.unlock();
}


void lsp::completion(file &File, screen &Screen) {
    if (running == false) { return; }

    std::string completionMsg;
    const char* json = "{\"jsonrpc\": \"2.0\",\"method\": \"textDocument/completion\",\"id\": 1,\"params\": {\"textDocument\": {\"uri\": \"\"},\"position\": {\"line\": 0,\"character\": 0}}}";
    std::string uri = "file://" + File.filePath;

    lock.lock();

    readAction = "complete";

    Document completion;
    completion.Parse(json);
    Document::AllocatorType& allocator = completion.GetAllocator();

    completion["params"]["textDocument"]["uri"].SetString(uri.c_str(), allocator);
    completion["params"]["position"]["line"].SetInt(Screen.cursorLine);
    completion["params"]["position"]["character"].SetInt(Screen.cursorChar);

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    completion.Accept(writer);

    printf("\n\n%s\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", sb.GetString());

    completionMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
    write(lspIn[1], completionMsg.c_str(), completionMsg.length());
    //printf("\n\n%s\n\n", response);

    lock.unlock();

}



void lsp::parseResponse(file &File, screen &Screen, cursor &Cursor, popup &Popup, int mode, char* response) {
    if (running == false) { return; }
    Document responseJson;
    responseJson.Parse(response);

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    responseJson.Accept(writer);

    //printf("\n\n%s\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", sb.GetString());


    //  Completion response, parse items and add to popup list
    if (responseJson["result"].IsObject() && responseJson["result"]["items"].IsArray()) {
        Popup.clr();

        for (Value::ConstValueIterator it = responseJson["result"]["items"].Begin(); it != responseJson["result"]["items"].End(); ++it) {
            const Value& items = *it;
            if (items["filterText"].IsString() == 1) {
                Popup.append(items["filterText"].GetString());
                Popup.start.push_back(items["textEdit"]["range"]["start"]["character"].GetInt());
                Popup.end.push_back(items["textEdit"]["range"]["end"]["character"].GetInt());
                Popup.line.push_back(items["textEdit"]["range"]["start"]["line"].GetInt());
                Popup.text.push_back(items["textEdit"]["newText"].GetString());
            }
        }

        Popup.print(Cursor);
        std::fflush(stdout);

    // FIXME: This is a mess, unfished implementation of how errors are printed to the screen

    //  (Diagnostic)/(Error responce), parse items and create error lines
    } else if (responseJson["method"].IsString() && strcmp(responseJson["method"].GetString(), "textDocument/publishDiagnostics") == 0) {
        struct error Err;
        std::string currentLine;

        printf("%s\n\n\n\n", sb.GetString());

        for (Value::ConstValueIterator it = responseJson["params"]["diagnostics"].Begin(); it != responseJson["params"]["diagnostics"].End(); ++it) {
            const Value& issue = *it;
            Err.line = issue["range"]["start"]["line"].GetInt();
            currentLine = File.vect[Err.line];
            Err.lineText = File.vect[Err.line];
            Err.start = issue["range"]["start"]["character"].GetInt();
            Err.end = issue["range"]["end"]["character"].GetInt();

            //Err.lineText = currentLine.substr(0, start) + "\x1b[9m" + currentLine.substr(start, end - start) + "\x1b[0m" + currentLine.substr(end) + "\x1b[2;3m  " + issue["message"].GetString();

            //printf("%s", Err.lineText.c_str());
            //Err.lineText.insert(Err.start, "\033[9m");
            //Err.lineText.insert(Err.end + 4, "\033[0m");
        
            Err.lineText.append("  \033[1;31m");
            Err.lineText.append(issue["message"].GetString());
            File.errMap[Err.line] = Err;
            
        }

        Screen.print(File, mode);
        Cursor.move();
        Popup.print(Cursor);
        std::fflush(stdout);
    }
}
#else

void lsp::start(file) {}
char* lsp::readJson() {
    char* retVal;
    return retVal;
}
void lsp::initialize() {}
void lsp::exit() {}
void lsp::didOpen(file &, std::string) {}
void lsp::didChange(file &, screen &, char) {}
void lsp::completion(file &, screen &) {}
void lsp::parseResponse(file &, screen &, cursor &, popup &, int, char*) {}

#endif
