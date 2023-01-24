#include <cstdio>
#include <vector>
#include <string>

#include "file.cpp"
#include "screen.cpp"

enum mode {
    command = 0,
    insert = 1
} Mode;

struct undoInstruction {
    int index;
    int line;
    int action;
    std::string text;
};

struct edit {
    int undoIndex = -1;
    std::string cmdLine2;
    std::vector<undoInstruction> undoStack;
    
    void insertMode(file &File, screen &Screen, cursor &Cursor, char c) {
        std::vector<int> action;
        std::string currentLine = File.vect[Screen.cursorLine];
        std::string lineBegin;
        std::string lineEnd;
        int index = Screen.cursorChar;
        int undoInsert;
        char character = File.vect[Screen.cursorLine][index];

        if (c == 127) { //  Backspace
            undoInsert = 1;
            File.vect[Screen.cursorLine].erase(index - 1, 1);
            Cursor.column--;
            Screen.cursorChar--;
            
        } else if (c == 10) { //  Newline
            undoInsert = 0;
            File.vect.insert(File.vect.begin() + Screen.cursorLine + 1, "");
            Cursor.row++;
            Screen.cursorLine++;
            Cursor.column = Cursor.offset;

        } else { //  Insert char
            undoInsert = 0;
            lineBegin = currentLine.substr(0, index);
            lineEnd = currentLine.substr(index);
            lineBegin.push_back(c);
            File.vect[Screen.cursorLine] = lineBegin + lineEnd;
            Screen.cursorChar++;
            Cursor.column++;

        }

        //  Create undo instruction, [line, index, (insert), char]
        undoInstruction Undo;

        Undo.line = Screen.cursorLine;
        Undo.index = Cursor.column - Cursor.offset;
        Undo.text = File.vect[Screen.cursorLine][index - 1];
        Undo.action = undoInsert;

        //  If the index is not at the end, remove everything under it to prevent breaking the stack
        if (!(++undoIndex == undoStack.size())) { undoStack.erase(undoStack.begin() + undoIndex, undoStack.end()); }

        undoStack.push_back(Undo);

    }

    void commandMode(file &File, screen &Screen, cursor &Cursor, char c) {
        if (c == 'i') { //  Insert mode
           Mode = insert; 

        } else if (c == 'x') { //  Delete char
            File.vect[Screen.cursorLine].erase(Cursor.column - Cursor.offset, 1);

        } else if (c == 'j') { //  Cursor down
            if (!(Cursor.row >= Screen.verticalSize)) { Cursor.row += 1; }
            Screen.cursorLine += 1;

        } else if (c == 'k') { //  Cursor up
            if (!(Cursor.row <= 1)) { Cursor.row -= 1; }
            if (!(Screen.cursorLine <= 1)) { Screen.cursorLine -= 1; }

        } else if (c == 'l') { //  Cursor right
            Screen.cursorChar++;
            Cursor.column++;

        } else if (c == 'h') { // Cursor left
            if (!(Cursor.column == Cursor.offset)) { 
                Cursor.column--; 
                Screen.cursorChar--;
            }

        } else if (c == 'w') { //  Write to file
            File.writeFile();
            Screen.cmdLine = "File hath been wroteth";

        } else if (c == 127) { //  Delete line
            File.vect.erase(File.vect.begin() + Screen.cursorLine);

        } else if (c == 't') {
            for (auto& it : undoStack) {
                printf("[%d, %d, %d, %s]\n", it.line, it.index, it.action, it.text.c_str());
            }
            exit(0);
        } else if (c == 'u') {
            if (undoIndex < 0) { return; } //  Do not undo if there is nothing to undo
            undo(File, Screen, Cursor);
        } else if (c == '$') {
            endOfTheLine(File, Screen, Cursor);
            Cursor.column = Cursor.offset;
            if (File.vect[Screen.cursorLine].length() < Screen.horizontalSize) {
                Cursor.column += File.vect[Screen.cursorLine].length();
            } else {
                Cursor.column += Screen.horizontalSize;
            }
        }

        if (Screen.cursorChar > File.vect[Screen.cursorLine].length()) {
           endOfTheLine(File, Screen, Cursor);
        }

    }

    void undo(file &File, screen &Screen, cursor &Cursor) {
        //  undoStack format [line, index, action (0-1), char]
        undoInstruction Undo = undoStack[undoIndex];
        std::string lineBegin = File.vect[Undo.line].substr(0, Undo.index);
        std::string lineEnd = File.vect[Undo.line].substr(Undo.index);
        Cursor.row = Undo.line + 1;
        Cursor.column = Cursor.offset + Undo.index;
        Screen.cursorLine = Cursor.row - 1;

        if (Undo.action == 1) {  //  Insert a char
            lineBegin.append(Undo.text);
            Cursor.column++;

        } else if (Undo.action == 0) { //  Delete a char
            lineBegin = File.vect[Undo.line].substr(0, Undo.index);
            lineEnd = File.vect[Undo.line].substr(Undo.index);
            lineBegin.pop_back();
            Cursor.column--;

        }
        File.vect[Undo.line] = lineBegin + lineEnd;
        undoIndex--;
    }

    void endOfTheLine(file &File, screen &Screen, cursor &Cursor) {
        Screen.rightLine = File.vect[Screen.cursorLine].length();
        Screen.cursorChar = Screen.rightLine;
        Screen.leftLine = Screen.rightLine - Screen.horizontalSize;

        if (Screen.leftLine < 0) {
            Screen.rightLine -= Screen.leftLine;
            Cursor.column = (Cursor.offset + Screen.horizontalSize) + Screen.leftLine;
            Screen.leftLine -= Screen.leftLine;
        }
    }
};
