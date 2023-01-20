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
        std::string currentLine = File.vect[Screen.cursorLine - 1];
        std::string lineBegin;
        std::string lineEnd;
        int index = Cursor.column - Cursor.offset;

        if (c == 127) { //  Backspace
            File.vect[Screen.cursorLine - 1].erase(index - 1, 1);
            Cursor.column--;
        } else if (c == 10) {
            File.vect.insert(File.vect.begin() + Screen.cursorLine, "");
            Cursor.row++;
            Screen.cursorLine++;
            Cursor.column = Cursor.offset;
        } else {
            lineBegin = currentLine.substr(0, index);
            lineEnd = currentLine.substr(index);
            lineBegin.push_back(c);
            File.vect[Screen.cursorLine - 1] = lineBegin + lineEnd;
            Cursor.column++;
        }
    }

    void commandMode(file &File, screen &Screen, cursor &Cursor, char c) {
        if (c == 'i') { //  Insert mode
           Mode = insert; 

        } else if (c == 'x') { //  Delete char
            File.vect[Screen.cursorLine - 1].erase(Cursor.column - Cursor.offset, 1);

        } else if (c == 'j') { //  Cursor down
            if (!(Cursor.row >= Screen.verticalSize)) { Cursor.row += 1; }
            Screen.cursorLine += 1;

        } else if (c == 'k') { //  Cursor up
            if (!(Cursor.row <= 1)) { Cursor.row -= 1; }
            if (!(Screen.cursorLine <= 1)) { Screen.cursorLine -= 1; }

        } else if (c == 'w') { //  Write to file
            File.writeFile();
            Screen.cmdLine = "File hath been wroteth";

        } else if (c == 127) { //  Delete line
            File.vect.erase(File.vect.begin() + Screen.cursorLine - 1);
        }

    }
};
