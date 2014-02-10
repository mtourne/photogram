#include "util.h"

std::string basename(const std::string& pathname) {
    return {std::find_if(pathname.rbegin(), pathname.rend(),
                         [](char c) { return c == '/'; }).base(),
            pathname.end()};
}

std::string remove_extension(const std::string& pathname) {
    string s = pathname;
    s.erase(s.find_last_of("."), string::npos);
    return s;
}
