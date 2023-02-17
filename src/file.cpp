#include "file.hpp"
#include "config.hpp"
#include <climits>
#include <unistd.h>

std::string file::openFile(std::string path) {
    std::string line;
    std::string retVal = "";
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("mite: this directory no longer exists\n");
        exit(0);
    }

    filePath = std::string(cwd) + "/" + path;

    //  If the file does not exist in cwd, check if it is a path
    //  to a file elsewhere
    if (access(filePath.c_str(), F_OK) != 0) {
        //  If its not a path to a file elsewhere, exit (FIXME for now)
        if (access(path.c_str(), F_OK) != 0) {
            printf("mite: no such file %s\n", path.c_str());
            exit(1);
        }
        //  If it is a path to a file elsewhere, remove the
        //  cwd from the path string
        filePath = std::string(path);
    }


    if (access(path.c_str(), R_OK) == -1) {
        return "permission denied: no read access";
    } else if (access(path.c_str(), W_OK) == -1) {
        retVal = "\033[1;31mwarning: opening read only file";
    }

    std::ifstream f(filePath);
    vect.clear();
    getLangId();

    while (getline(f, line)) {
        vect.push_back(line);
    }

    f.close();
    return retVal;
} 

void file::writeFile() {
    std::ofstream f(filePath);

    for (auto& it : vect) {
        f << it + '\n';
    }
}

void file::toString() {
    for (const auto& it : vect) {
        string.append(it + "\r\n");
    }
}

void file::getLangId() {
    std::string fileEnd = std::filesystem::path(filePath).extension();

    if (fileEnd == ".cpp" || fileEnd == ".hpp") {
        languageServer = cppServer;
        languageId = "cpp";
    } else if (fileEnd == ".c" || fileEnd == ".h") {
        languageServer = cServer;
        languageId = "c";
    } else if (fileEnd == ".rs") {
        languageServer = rustServer;
        languageId = "rust";
    }
}
