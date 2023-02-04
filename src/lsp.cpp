#define RAPIDJSON_ASSERT(x)

#include <cstdlib>
#include <rapidjson/stringbuffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <string>
#include <string.h>
#include <iostream>
#include <climits>

#include "screen.cpp"
#include "file.cpp"

#define err(...) fprintf(stderr, __VA_ARGS__)

using namespace rapidjson;

static std::mutex lock;

struct lsp {
    std::string jsonString;
    std::string jsonOut;
    std::string readAction;
    int jOut;
    int version = 1;
    int lspIn[2];
    int lspOut[2];
    int lspErr[2];

    


    void start(std::string languageServer) {
        int ret;
        int jsonSize = 0;

        if (pipe(lspIn) == -1) { err("pipe 1 failed"); }
        if (pipe(lspOut) == -1) { err("pipe 1 failed"); }
        if (pipe(lspErr) == -1) { err("pipe 1 failed"); }

        int pid = fork();

        if (pid == 0) {
            dup2(lspIn[0], 0);
            dup2(lspOut[1], 1);
            dup2(lspErr[1], 2);
            char *args[] = { (char *)languageServer.c_str(), NULL };
            execvp(languageServer.c_str(), args);
        }
    }


    char* readJson() {
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



    void initialize() {
        std::string initMsg;
        std::string initNotif;
        Document initializeMsg;
        Document initNotification;
        initializeMsg.SetObject();
        initNotification.SetObject();

        Document::AllocatorType& allocator = initializeMsg.GetAllocator();
        Document::AllocatorType& allocator2 = initNotification.GetAllocator();
        size_t sz = allocator.Size();
        size_t sz2 = allocator2.Size();

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


    void exit() {
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


    void didOpen(file &File, std::string language) {
        std::string openMsg;
        const char* json = "{\"jsonrpc\": \"2.0\",\"method\": \"textDocument/didOpen\",\"params\":{\"textDocument\": {\"uri\": \"\",\"languageId\": \"\",\"version\": 1,\"text\": \"\"}}}";
        std::string uri = "file://" + File.filePath;
        std::string text = "int main() { \r\nint dogg = 1;\r\ndog";

        lock.lock();
        
        Document didOpen;
        didOpen.Parse(json);
        Document::AllocatorType& allocator = didOpen.GetAllocator();
        size_t sz = allocator.Size();

        didOpen["params"]["textDocument"]["uri"].SetString(uri.c_str(), allocator);
        didOpen["params"]["textDocument"]["languageId"].SetString(language.c_str(), allocator);
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


    void didChange(file &File, screen &Screen, char c) {
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

        // ES MUY NO BUENO
        // REPAIR THE SON OF A SHITTER
        if (c == 13) { //  Newline insert 
            didChange["params"]["contentChanges"][0]["range"]["start"]["line"].SetInt(line);
            didChange["params"]["contentChanges"][0]["range"]["start"]["character"].SetInt(0);
            didChange["params"]["contentChanges"][0]["range"]["end"]["line"].SetInt(line);
            didChange["params"]["contentChanges"][0]["range"]["end"]["character"].SetInt(0);
            didChange["params"]["contentChanges"][0]["text"].SetString("\r\n", allocator);

            didChange["params"]["contentChanges"][1].RemoveAllMembers();
        } else {
            didChange["params"]["contentChanges"][0]["range"]["start"]["line"].SetInt(line);
            didChange["params"]["contentChanges"][0]["range"]["start"]["character"].SetInt(0);
            didChange["params"]["contentChanges"][0]["range"]["end"]["line"].SetInt(line + 1);
            didChange["params"]["contentChanges"][0]["range"]["end"]["character"].SetInt(0);
            didChange["params"]["contentChanges"][0]["text"].SetString("", allocator);

            didChange["params"]["contentChanges"][1]["range"]["start"]["line"].SetInt(line);
            didChange["params"]["contentChanges"][1]["range"]["start"]["character"].SetInt(0);
            didChange["params"]["contentChanges"][1]["range"]["end"]["line"].SetInt(line);
            didChange["params"]["contentChanges"][1]["range"]["end"]["character"].SetInt(0);
            didChange["params"]["contentChanges"][1]["text"].SetString(text.c_str(), allocator);
        } 


        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        didChange.Accept(writer);

        printf("\n\n%s\n\n", sb.GetString());

        didChangeMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
        write(lspIn[1], didChangeMsg.c_str(), didChangeMsg.length());
        //readJson();
        lock.unlock();
    }


    void completion(file &File, screen &Screen, popup &Popup) {
        std::string completionMsg;
        const char* json = "{\"jsonrpc\": \"2.0\",\"method\": \"textDocument/completion\",\"id\": 1,\"params\": {\"textDocument\": {\"uri\": \"\"},\"position\": {\"line\": 0,\"character\": 0}}}";
        char* response;
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

        //printf("\n\n%s\n\n", sb.GetString());

        completionMsg = "Content-Length: " + std::to_string(std::string(sb.GetString()).length()) + "\r\n\r\n" + sb.GetString();
        write(lspIn[1], completionMsg.c_str(), completionMsg.length());
        //printf("\n\n%s\n\n", response);

        lock.unlock();

    }



    void parseResponse(file &File, screen &Screen, cursor &Cursor, popup &Popup, int mode, char* response) {
        Document responseJson;
        responseJson.Parse(response);

        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        responseJson.Accept(writer);

        //printf("\n\n%s\n\n", sb.GetString());


        //  Completion response, parse items and add to popup list
        if (responseJson["result"].IsObject() && responseJson["result"]["items"].IsArray()) {
            Popup.list.clear();

            for (Value::ConstValueIterator it = responseJson["result"]["items"].Begin(); it != responseJson["result"]["items"].End(); ++it) {
                const Value& items = *it;
                Popup.append(items["filterText"].GetString());
            }

            Popup.print(Cursor);
            std::fflush(stdout);

        //  (Diagnostic) Error responce, parse items and create error lines
        } else if (responseJson["method"].IsString() && strcmp(responseJson["method"].GetString(), "textDocument/publishDiagnostics") == 0) {
            struct error Err;
            std::string currentLine;
            int start;
            int end;

            //printf("%s", sb.GetString());

            for (Value::ConstValueIterator it = responseJson["params"]["diagnostics"].Begin(); it != responseJson["params"]["diagnostics"].End(); ++it) {
                const Value& issue = *it;
                Err.line = issue["range"]["start"]["line"].GetInt();
                currentLine = File.vect[Err.line];
                start = issue["range"]["start"]["character"].GetInt();
                end = issue["range"]["end"]["character"].GetInt();

                Err.lineText = currentLine.substr(0, start) + "\x1b[9m" + currentLine.substr(start, end) + "\x1b[0m" + currentLine.substr(end) + "\x1b[2;3m  " + issue["message"].GetString();
                //printf("%s", Err.lineText.c_str());
                File.errVect.push_back(Err);
                
            }

            Screen.print(File, Popup, mode);
            Cursor.move();
            Popup.print(Cursor);
            std::fflush(stdout);
        }
    }
};


/*
int main() {
    lsp Lsp;
    cursor Cursor;
    file File;

    File.string = "#include <string>\r\nint main{\r\nstd::strin}";
    File.filePath = "/home/coffee/test.cpp";
    Cursor.row = 2;
    Cursor.column = 2;
    std::string hi;

    std::thread lisp([&]() {
        Lsp.start("clangd");
    });
    lisp.detach();

    Lsp.initialize();
    Lsp.didOpen("/home/coffee/test.cpp", "cpp");
    Lsp.completion(File, Cursor);

    printf("hoi");
    std::cin >> hi;
    
}
*/
