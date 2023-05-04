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
    line.clear();
    text.clear();
    start.clear();
    end.clear();
}

void screen::clearAnnotations() {
    for (auto& it : fileAnnotations) {
        fileAnnotations[it.first].erase(std::remove_if(it.second.begin(), it.second.end(), [&](annotations annotate){
            return annotate.localIndex == true;
        }), it.second.end());
    }
}

void screen::init(winsize size) {
    verticalSize = size.ws_row - 1;
    horizontalSize = size.ws_col;
    bottomLine = verticalSize;
    rightLine = horizontalSize - 8;
}

//  FIXME: This fucntion is a mess
void screen::print(file File, int mode) {
    int line = 0;
    int size;
    int offset = 0;
    int startIndex;
    int endIndex;
    std::string errorText;
    std::string lineText;
    std::string viewableLine;

    printf("\033[H\033[J%s", RESET);

    for (auto& it : File.vect) {
        offset = 0;
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
            errorText.clear();
            size = rightLine - (horizontalSize - 8); //  FIXME: is this not what leftline is?
            viewableLine = lineText.substr(size, horizontalSize - 8);
            offset = 0;

            if (mode == 0) {
                for (auto& it : fileAnnotations[line]) {
                    if (it.localIndex == false) {
                        startIndex = it.start - File.vectIndicies[line - 1];
                        endIndex = it.end - File.vectIndicies[line - 1];
                    } else {
                        startIndex = it.start;
                        endIndex = it.end;
                    }

                    //  If the start and end index are offscreen to the left, skip to save
                    //  time not going through if statements that will fail
                    /*
                    if (startIndex < size && endIndex < size) {
                        continue;

                    //  Same if they're offscreen to the right, different statement so the 
                    //  line isnt awfully long and hard to read
                    } else if (startIndex > size + (horizontalSize - 8) && endIndex > size + (horizontalSize - 8)) {
                        continue;
                    }
                    */


                    //  If the starting index is not offscreen to the left, AND the index is not 
                    //  offscreen to the right, then the index is within the area that is about
                    //  to be printed, so we insert it normally
                    if (startIndex > size && startIndex < size + (horizontalSize - 8)) {
                        viewableLine.insert(startIndex + offset - size, it.first.c_str());
                        offset += it.first.size();

                    //  If the start index is offscreen to the left, AND the end index is 
                    //  not offscreen to the left, then the color  will not be displayed as 
                    //  the starting color ascii code is offscreen. So we insert that color 
                    //  code at the beginning of the line so that it is displayed
                    } else if (startIndex <= size && endIndex > size) {
                        viewableLine.insert(0 + offset, it.first.c_str());
                        offset += it.first.size();
                    }


                    //  If the end index is not offscreen to the left, AND not offscreen to 
                    //  the right, insert the end seqence normally
                    if (endIndex > size && endIndex < size + (horizontalSize - 8)) {
                        viewableLine.insert(endIndex + offset - size, it.last);
                        offset += it.last.size();

                    //  If the end index is offscreen to the right, add it to the end of the 
                    //  printed line. Just for good measure I guess
                    //
                    //  FIXME: maybe check for a startIndex to avoid printing random RESET 
                    //  sequences 
                    } else if (endIndex > horizontalSize - 8) {
                        viewableLine.append(it.last);
                        offset += it.last.size();
                    }
                    //printf("\toffset: %i | startIndex: %i | endIndex: %i | size: %i | line: %s\n", offset, startIndex, endIndex, size, viewableLine.c_str());
                    errorText = it.errColor + it.errMsg;
                }
            }

            printf("%-5d| %s\t%s%s\n", line, viewableLine.c_str(), errorText.c_str(), RESET);
        } else {
            printf("%-5d| %s\n", line, RESET);
        }

        if (line > bottomLine) {
            break;
        }
    }
    
    printf("\033[1m%s%s", modes[mode].c_str(), cmdLine.c_str());
}
