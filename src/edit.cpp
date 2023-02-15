#include "edit.hpp"


void edit::insertMode(file &File, screen &Screen, cursor &Cursor, popup &Popup, char &c) {
    std::vector<int> action;
    std::string currentLine = File.vect[Screen.cursorLine];
    std::string lineBegin;
    std::string lineEnd;
    int index = Screen.cursorChar;
    int undoInsert;
    int length;

    undoInstruction Undo;

    if (c == 127) { //  Backspace
        undoInsert = 1;
        File.vect[Screen.cursorLine].erase(index - 1, 1);
        Cursor.column--;
        Screen.cursorChar--;
        Undo.text = File.vect[Screen.cursorLine][index - 1];
        Undo.index = Cursor.column - Cursor.offset - 1;
        
    } else if (c == 13) { //  Newline
        if (Popup.list.size() > 0) { //  Do a completion instead of a newline if possible
            undoInsert = 0;
            length = Popup.end[Popup.listIndex] - Popup.start[Popup.listIndex];
            File.vect[Screen.cursorLine].replace(Popup.start[Popup.listIndex], length, Popup.text[Popup.listIndex]);
            Screen.cursorChar += Popup.text[Popup.listIndex].length() - length;
            Cursor.column = Screen.cursorChar + 8;
            Popup.clr();
            //  Change c to something other than 13, otherwise Lsp.didChange()
            //  will tell the server to add a newline instead of the completion
            c = 10;
            Undo.text = Popup.text[Popup.listIndex];
            Undo.index = Popup.start[Popup.listIndex];
        } else {
            undoInsert = 0;
            File.vect.insert(File.vect.begin() + Screen.cursorLine + 1, "");
            Cursor.row++;
            Screen.cursorLine++;
            Cursor.column = Cursor.offset;
            Screen.cursorChar = 0;
            Undo.text = "\r\n";
        }

    } else { //  Insert char
        undoInsert = 0;
        lineBegin = currentLine.substr(0, index);
        lineEnd = currentLine.substr(index);
        lineBegin.push_back(c);
        File.vect[Screen.cursorLine] = lineBegin + lineEnd;
        Screen.cursorChar++;
        if (!(Cursor.column == Screen.horizontalSize)) { Cursor.column++; }
        Undo.text = c;
        Undo.index = Cursor.column - Cursor.offset - 1;
    }

    //  Create undo instruction, [line, index, (insert), char]

    Undo.line = Screen.cursorLine;
    Undo.action = undoInsert;

    //  If the index is not at the end, remove everything under it to prevent breaking the stack
    if (!(++undoIndex == undoStack.size())) { undoStack.erase(undoStack.begin() + undoIndex, undoStack.end()); }

    undoStack.push_back(Undo);

}

int edit::commandMode(file &File, screen &Screen, cursor &Cursor, popup &Popup, mode &Mode, char &c) {
    int retVal = 0;
    if (c == 'i') { //  Insert mode
       Mode = insert; 

    } else if (c == 'x') { //  Delete char
        File.vect[Screen.cursorLine].erase(Cursor.column - Cursor.offset, 1);
        retVal = 1;

    } else if (c == 'j') { //  Cursor down
        if (!(Cursor.row >= Screen.verticalSize)) { Cursor.row += 1; }
        Screen.cursorLine += 1;

    } else if (c == 'k') { //  Cursor up
        if (!(Cursor.row <= 1)) { Cursor.row -= 1; }
        if (!(Screen.cursorLine < 1)) { Screen.cursorLine -= 1; }

    } else if (c == 'l') { //  Cursor right
        if (Screen.cursorChar <= File.vect[Screen.cursorLine].length()) { Screen.cursorChar++; }
        if (!(Cursor.column == Screen.horizontalSize)) { Cursor.column++; }

    } else if (c == 'h') { // Cursor left
        if (!(Cursor.column == Cursor.offset)) { Cursor.column--; }
        if (Screen.cursorChar > 0) { Screen.cursorChar--; }

    } else if (c == 'w') { //  Write to file
        File.writeFile();
        Screen.cmdLine = "File hath been wroteth";

    } else if (c == 127) { //  Delete line
        File.vect.erase(File.vect.begin() + Screen.cursorLine);

    } else if (c == 't') {
        for (auto& it : undoStack) {
            printf("\n[%d] [%d] [%d] [%s]\n", it.line, it.index, it.action, it.text.c_str());
        }
        exit(0);

    } else if (c == 'u') {
        if (undoIndex < 0) { return 0; } //  Do not undo if there is nothing to undo
        undo(File, Screen, Cursor);
        retVal = 1;

    } else if (c == '$') {
        endOfTheLine(File, Screen, Cursor);
        Cursor.column = Cursor.offset + Screen.horizontalSize;

        //  If the line is shorter than the screen
        //  the terminal cursor will be in the wrong spot
        if ((int) File.vect[Screen.cursorLine].length() < Screen.horizontalSize) {  
            Cursor.column = Cursor.offset + File.vect[Screen.cursorLine].length();
        }
    }

    if (Screen.cursorChar > File.vect[Screen.cursorLine].length()) {
       endOfTheLine(File, Screen, Cursor);
    }

    return retVal;
}

void edit::undo(file &File, screen &Screen, cursor &Cursor) {
    //  undoStack format [line, index, action (0-1), char]
    undoInstruction Undo = undoStack[undoIndex];
    std::string lineBegin = File.vect[Undo.line].substr(0, Undo.index);
    std::string lineEnd = File.vect[Undo.line].substr(Undo.index);
    Cursor.row = Undo.line + 1;
    Cursor.column = Cursor.offset + Undo.index;
    Screen.cursorLine = Cursor.row - 1;

    if (Undo.action == 1) {  //  Insert a char
        lineBegin.append(Undo.text.c_str());
        Cursor.column++;

    } else if (Undo.action == 0) { //  Delete a char
        File.vect[Undo.line].erase(Undo.index, Undo.text.length());
        Cursor.column--;

    }
    undoIndex--;
}

void edit::endOfTheLine(file &File, screen &Screen, cursor &Cursor) {
    Screen.rightLine = File.vect[Screen.cursorLine].length();
    Screen.cursorChar = Screen.rightLine;
    Screen.leftLine = Screen.rightLine - (Screen.horizontalSize - 8);

    if (Screen.leftLine < 0) {
        Screen.rightLine -= Screen.leftLine;
        Cursor.column = Screen.horizontalSize + Screen.leftLine;
        Screen.leftLine -= Screen.leftLine;
    }
}
