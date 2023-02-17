#include "screen.hpp"


void cursor::move() {
    printf("\x1b[%d;%dH", row, column);
}


void popup::print(cursor &Cursor) {
    int line = Cursor.row + 1;
    int index = 0;

    printf("\x1b[B");
    for (auto& it : list) {
        index++;
        if (listIndex == index) { color = "\x1b[42m"; } //  Highlighted item
        printf("%s %s%s\x1b[%d;%dH", color.c_str(), it.c_str(), std::string(length - it.length() + 1, ' ').c_str(),++line, Cursor.column); //  Print item, move back and down, print again
        color = "\x1b[41m"; //  Dehighlight item

        if (index == bottomLine) { break; }
    }
    printf("\x1b[0m");
    Cursor.move(); //  Move cursor back to original position
}


void popup::append(std::string value) {
    if ((int) value.length() > length) {
        length = value.length();
    }
    list.push_back(value);
}

void popup::clr() {
    list.clear();
    text.clear();
    start.clear();
    end.clear();
}


void screen::init(winsize size) {
    verticalSize = size.ws_row - 1;
    horizontalSize = size.ws_col;
    bottomLine = verticalSize;
    rightLine = horizontalSize - 8;
}

//  FIXME: This fucntion is a mess
void screen::print(file File, popup &Popup, int mode) {
    int line = 0;
    int size;
    int start;
    int end;
    std::string lineText;
    std::string viewableLine;

    printf("\033[H\033[J%s", RESET.c_str());

    for (auto& it : File.vect) {
        start = -1;
        end = -1;
        lineText = it;
        if (File.errMap.count(line) > 0 && mode == 0) { lineText = File.errMap[line].lineText; }
        line++;
        


        if (cursorLine > bottomLine) { //  Scrolling down
            bottomLine++;
            topLine++;
        } else if (cursorLine < topLine) { //  Scrolling up
            bottomLine--;
            topLine--;
        }

        if (cursorChar > rightLine) {
            rightLine++;
            leftLine++;
        } else if (cursorChar < leftLine) {
            rightLine--;
            leftLine--;
        }

        if (line < topLine) {
            continue;
        }

        if ((int)lineText.length() > rightLine - (horizontalSize - 8)) {
            if (File.errMap.count(line - 1) > 0) {
                start = File.errMap[line - 1].start;
                end = File.errMap[line - 1].end;
            }
            size = rightLine - (horizontalSize - 8);
            viewableLine = lineText.substr(size, horizontalSize - 8);

            //  Error stuff
            if (!(start == -1) && mode == 0) {
                if (start > size && start < size + (horizontalSize - 8)) {
                    viewableLine.insert(start - size, "\x1b[9m");
                } else if (start <= size && end > size) {
                    viewableLine.insert(0, "\x1b[9m");
                }

                if (end > horizontalSize - 8) {
                    viewableLine.append(RESET);
                } else if (end > size && end < size + (horizontalSize - 8)) {
                    viewableLine.insert(end - size + 4, RESET);
                }
            }

            printf("%-5d| %s%s\n", line, viewableLine.c_str(), RESET.c_str());
        } else {
            printf("%-5d| %s\n", line, RESET.c_str());
        }

        if (line > bottomLine) {
            break;
        }
    }
    printf("\033[1m-- %s -- %s", modes[mode].c_str(), cmdLine.c_str());
}
