#pragma once

#include <string>

class File {
private:
    std::ofstream file_;

public:
    File() = default;
    ~File() {
        close();
    }

    bool open(const std::string& filename) {
        file_.open(filename, std::ios::out);
        return file_.is_open();
    }

    void close() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void line(const std::string& data) {
        if (file_.is_open()) {
            file_ << data << endl;
        }
    }
};
