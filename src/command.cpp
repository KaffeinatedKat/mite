#include "command.hpp"

int command::get(screen &Screen, cursor &Cursor, std::string &cmd) {
    int cursorRow;
    int cursorColumn;
    char c;

    //  Save the current terminal cursor's position before moving
    //  it to where the command input is typed so it can be moved
    //  back afterwards
    cursorRow = Cursor.row;
    cursorColumn = Cursor.column;

    //  Move the terminal cursor to where the command is being
    //  displayed at the bottom of the screen
    Cursor.row = Screen.horizontalSize;
    Cursor.column = 0;
    Cursor.move();

    //  Clear the line and print the current contents of the command 
    //  string
    printf("\x1b[2K:%s", cmd.c_str());
    std::fflush(stdout);

    while (read(STDIN_FILENO, &c, 1)) {

        if (c == 27) {  //  ESC
            //  If ESC is pressed we want to exit the command line
            //  and not execute anything.
            //
            //  Move the terminal cursor back to the saved coordinates 
            //  and return 0 to indicate no command to execute
            Cursor.row = cursorRow;
            Cursor.column = cursorColumn;
            Cursor.move();
            return 0;
        } else if (c == 127) { //  Backspace
            //  If backspace is pressed we want to remove the last character
            //  from the command string instead of inserting the delete character
            //  which is not what we want
            cmd.pop_back();
        } else if (c == 13) { //  Enter
            //  If enter is pressed we want to restore the terminal cursor 
            //  and save the command to be executed
            //
            //  Move the cursor back to the saved position, then return 1 to
            //  indicate that we have a command to be executed
            Cursor.row = cursorRow;
            Cursor.column = cursorColumn;
            Cursor.move();
            return 1;
        } else {
            //  If nothing else goes through, then it is a char we want to
            //  add to the command string
            cmd.push_back(c);
        }

        //  Move the terminal cursor to where the input is and then clear the
        //  line and print the current version of the command string
        Cursor.row = Screen.horizontalSize;
        Cursor.column = 0;
        Cursor.move();
        printf("\x1b[2K:%s", cmd.c_str());
        std::fflush(stdout);
    }

    //  Code should never get here, but this prevents the warning
    //  "not all cases return in return function". Return 0 in case
    //  it does somehow get here to indicate no command to be executed
    return 0;
}


void command::execute(file &File, screen &Screen, lsp &Lsp, std::string command) {
    //  FIXME: do not exit on 'q' if the file has been modified and
    //  not saved, 'q!' to override 
    if (command == "w") {
        File.writeFile();
        Screen.cmdLine = "File hath been wroteth";
    } else if (command == "q") {
        quit(Lsp);
    } else if (command == "wq") {
        File.writeFile();
        quit(Lsp);
    }
}
