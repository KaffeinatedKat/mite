#include "edit.hpp"


int edit::insertMode(file &File, screen &Screen, cursor &Cursor, popup &Popup, char &c) {
    std::vector<int> action;
    std::string currentLine = File.vect[Screen.cursorLine];
    std::string lineBegin;
    std::string lineEnd;
    int index = Screen.cursorChar;
    int undoInsert;
    int length;

    undoInstruction Undo;

    if (c == 127) { //  Backspace

        //  If backspace is pressed at the beginning of a line then
        //  we want to combine the current and top lines together.
        //
        //  Move the cursor to the end of the line above, then
        //  move the contents of the line below to the end of the 
        //  current line and delete the line below
        //
        //  FIXME: can delete lines into the shadow realm if
        //  deleting from the beginning of the top line
        if (Screen.cursorChar <= 0) {
            undoInsert = 1;
            Screen.cursorLine--;
            Cursor.row--;
            endOfTheLine(File, Screen, Cursor);
            File.vect[Screen.cursorLine].append(File.vect[Screen.cursorLine + 1]);
            File.vect.erase(File.vect.begin() + Screen.cursorLine + 1);

        //  If not then we want to do a normal delete.
        //
        //  Delete the character behind the cursor, then move the
        //  cursor back one column to reflect the newly deleted
        //  character
        } else {
            undoInsert = 1;
            File.vect[Screen.cursorLine].erase(index - 1, 1);
            Cursor.column--;
            Screen.cursorChar--;
            Undo.text = File.vect[Screen.cursorLine][index - 1];
            Undo.index = Cursor.column - Cursor.offset - 1;
        }


    } else if (c == 13) { //  Newline (enter has been pressed)

        //  If the popup list contains items then we want to do an 
        //  autocomplete instead of a newline. 
        //
        //  We replace the range of the autocomplete's end index minus the 
        //  start index in the current line with the autocomplete text.
        //  Then add the length of the autocomplete text minus the length of 
        //  the ranges which puts the cursor at the end of the newly inserted
        //  text. Then clear the autocomplete list to prevent autocompleting the
        //  same text again.
        if (Popup.list.size() > 0) {
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

        //  If not, then we want to insert a newline by splitting the current
        //  line into 2 lines at the cursor. This both works for splitting 
        //  a line and also creating new empty lines
        //
        //  Make the current line only include the text after the cursor's 
        //  current position, then add a new line below the current line
        //  with all the text after the cursor, move the cursor down to 
        //  reflect these changes
        } else {
            undoInsert = 0;
            File.vect[Screen.cursorLine] = currentLine.substr(0, Screen.cursorChar);
            File.vect.insert(File.vect.begin() + Screen.cursorLine + 1, currentLine.substr(Screen.cursorChar));
            Cursor.row++;
            Screen.cursorLine++;
            Cursor.column = Cursor.offset;
            Screen.cursorChar = 0;
            Undo.text = "\r\n";
        }

    //  If none of the other if statements have gone through, then
    //  it is probably a character we want to insert into the text.
    //
    //  Split the current line into 2 substrings, one before and one
    //  after the cursor. Push the new char to the end of the beginning
    //  substring then set the current line to the combination of the
    //  2 substrings
    //
    //  FIXME: This should use string.insert instead of substring
    } else {
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

    //  Undo instructions are created above by each case and follow the format 
    //  of [line, index, mode, text]. Here we add the new instruction to the 
    //  undo stack.
    Undo.line = Screen.cursorLine;
    Undo.action = undoInsert;

    //  If the undo index is not at the end of the stack, remove everything 
    //  under it to prevent breaking the text. Otherwise you can undo into
    //  text that may or may not exist on the document breaking things in a
    //  weird way.
    if (!(++undoIndex == (int) undoStack.size())) { undoStack.erase(undoStack.begin() + undoIndex, undoStack.end()); }

    undoStack.push_back(Undo);

    //  Return 1 to signify a change in the document to decide wether or 
    //  not to call Lsp.didChange();
    return 1;
}

int edit::commandMode(file &File, screen &Screen, cursor &Cursor, mode &Mode, char &c) {
    int retVal = 0;

    if (c == 'i') { //  Insert mode
        Mode = insert; 

    } else if (c == 'a') {
        Cursor.column++;
        Screen.cursorChar++;
        Mode = insert;

    } else if (c == 'x') { //  Delete char at current index
        File.vect[Screen.cursorLine].erase(Cursor.column - Cursor.offset, 1);
        retVal = 1;

    } else if (c == 'j') { //  Cursor down
        //  Do not move the terminal's cursor "out of bounds" when scrolling
        //  downwards, or it will not move up again when scrolling back up.
        //  Check if it's at the bottom and don't move it if it is
        if (!(Cursor.row >= Screen.verticalSize)) { Cursor.row += 1; }
        Screen.cursorLine += 1;

    } else if (c == 'k') { //  Cursor up
        //  Check if the cursor is at the topline to prevent you from
        //  scrolling up past the first line, as there is no text up there
        if (!(Cursor.row <= 1)) { Cursor.row -= 1; }
        if (!(Screen.cursorLine < 1)) { Screen.cursorLine -= 1; }

    } else if (c == 'l') { //  Cursor right
        if (Screen.cursorChar <= (int) File.vect[Screen.cursorLine].length()) { Screen.cursorChar++; }
        if (!(Cursor.column == Screen.horizontalSize)) { Cursor.column++; }

    } else if (c == 'h') { // Cursor left
        //  Check if the terminal cursor is at the offset value to prevent
        //  you from scrolling into the line counter
        if (!(Cursor.column == Cursor.offset)) { Cursor.column--; }
        //  Check if the cursor is the beginning of a line to prevent the
        //  cursor from being at a negitive index likely causing a crash at
        //  some point
        if (Screen.cursorChar > 0) { Screen.cursorChar--; }

    } else if (c == 'w') { //  Write to file
        File.writeFile();
        Screen.cmdLine = "File hath been wroteth";

    } else if (c == 127) { //  Delete line
        //  FIXME: this shouldn't exist, does not work with lsp. Needs to
        //  be replaced with 'dd' when the command line is implemented 
        File.vect.erase(File.vect.begin() + Screen.cursorLine);

    } 

#ifdef DEBUG
    else if (c == 't') { //  Debug print the undo stack 
        for (auto& it : undoStack) {
            printf("\n[%d] [%d] [%d] [%s]\n", it.line, it.index, it.action, it.text.c_str());
        }
        exit(0);

    } 
#endif

    else if (c == 'u') { //  Undo
        //  If the undo index is 0, then when you try to undo you
        //  get a std::out_of_range, check for this to prevent a crash.
        //
        //  Return 0 to signify no change to the document
        if (undoIndex < 0) { return 0; }
        undo(File, Screen, Cursor);

        //  Undo was exectued and the document has been changed. Return 
        //  value of 1 to reflect that
        retVal = 1;

    } else if (c == '$') { //  Move cursor to the end of the current line
        endOfTheLine(File, Screen, Cursor);
        //  Move the cursor to the end of the screen
        Cursor.column = Cursor.offset + Screen.horizontalSize;

        //  If the current line is shorter than the horizontal size
        //  of the terminal the terminal cursor will be in the wrong spot.
        // 
        //  Move the terminal cursor to the end of the current line accouting
        //  for the offset of the line counter
        if ((int) File.vect[Screen.cursorLine].length() < Screen.horizontalSize) {  
            Cursor.column = Cursor.offset + File.vect[Screen.cursorLine].length();
        }
    }

    //  If the cursor's position is greater than the current line (eg. moving 
    //  up/down to a shorter line from a longer one), move the cursor to the 
    //  end of the current line to account for it.
    if (Screen.cursorChar > (int) File.vect[Screen.cursorLine].length()) {
       endOfTheLine(File, Screen, Cursor);
    }

    return retVal;
}

void edit::undo(file &File, screen &Screen, cursor &Cursor) {
    //  undoStack instruction format [line, index, action (0-1), char]
    //
    //  FIXME: this is weird and half working. Insert action does not work
    //  and needs to work with newlines 

    undoInstruction Undo = undoStack[undoIndex];
    std::string lineBegin = File.vect[Undo.line].substr(0, Undo.index);
    std::string lineEnd = File.vect[Undo.line].substr(Undo.index);
    Cursor.row = Undo.line + 1;
    Cursor.column = Cursor.offset + Undo.index;
    Screen.cursorLine = Cursor.row - 1;

    //  FIXME: this doesnt do anything?
    if (Undo.action == 1) {  //  Insert action
        //  Append the undo text to the end of the beginning
        //  substring
        lineBegin.append(Undo.text.c_str());
        Cursor.column++;

    } else if (Undo.action == 0) { //  Delete action
        //  Remove the length of the undo text from the undo line
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
