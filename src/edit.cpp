#include <cstdio>
#include <vector>
#include <string>

#include "file.cpp"
#include "screen.cpp"

enum mode {
    command = 0,
    insert = 1
} Mode;

struct edit {
    std::string cmdLine2;
    


    void insertMode(file &File, screen &Screen, cursor &Cursor, char c) {
        std::string currentLine = File.vect[Screen.cursorPos - 1];
        std::string lineBegin;
        std::string lineEnd;
        int index = Cursor.column - Cursor.offset;

        if (c == 127) {
            File.vect[Screen.cursorPos - 1].erase(index, 1);
        } else {
            lineBegin = currentLine.substr(0, index);
            lineEnd = currentLine.substr(index);
            lineBegin.push_back(c);
            File.vect[Screen.cursorPos - 1] = lineBegin + lineEnd;
            Cursor.column++;
        }
    }

    void commandMode(file &File, screen &Screen, cursor &Cursor, char c) {
        if (c == 'i') {
           Mode = insert; 
        } else if (c == 'j') {
            if (!(Cursor.row >= Screen.verticalSize)) { Cursor.row += 1; }
            Screen.cursorPos += 1;
        } else if (c == 'k') {
            if (!(Cursor.row <= 1)) { Cursor.row -= 1; }
            if (!(Screen.cursorPos <= 1)) { Screen.cursorPos -= 1; }
        } else if (c == 'w') {
            File.writeFile();
            Screen.cmdLine = "File hath been wroteth";
        } else if (c == 127) {
            File.vect.erase(File.vect.begin() + Screen.cursorPos - 1);
        }
    }
};
