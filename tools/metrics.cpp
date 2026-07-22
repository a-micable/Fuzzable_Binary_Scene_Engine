#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

namespace fs = std::filesystem;

namespace {
bool hasSourceExtension(const fs::path& path) {
    static const std::set<std::string> extensions{
        ".cpp", ".cc", ".cxx", ".hpp", ".hh", ".hxx", ".h", ".cmake", ".txt", ".md"
    };
    return extensions.count(path.extension().string()) != 0;
}

bool ignoredPath(const fs::path& path) {
    const auto text = path.generic_string();
    return text.find("/.git/") != std::string::npos ||
           text.find("/build/") != std::string::npos ||
           text.find("/fuzz/seed_corpus/") != std::string::npos;
}

std::string trim(std::string line) {
    auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first >= last ? std::string{} : std::string(first, last);
}

bool boilerplateLine(const std::string& line, bool& blockComment) {
    auto text = trim(line);
    if (text.empty()) return true;
    if (blockComment) {
        if (text.find("*/") != std::string::npos) blockComment = false;
        return true;
    }
    if (text.rfind("//", 0) == 0) return true;
    if (text.rfind("/*", 0) == 0) {
        if (text.find("*/") == std::string::npos) blockComment = true;
        return true;
    }
    if (text == "{" || text == "}" || text == "};" || text == "};") return true;
    return false;
}
}

int main(int argc, char** argv) {
    fs::path root = argc > 1 ? fs::path(argv[1]) : fs::current_path();
    std::size_t files = 0;
    std::size_t workingLines = 0;
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        const auto path = entry.path();
        if (ignoredPath(path) || !hasSourceExtension(path)) continue;
        std::ifstream in(path);
        if (!in) continue;
        ++files;
        bool blockComment = false;
        std::string line;
        while (std::getline(in, line)) {
            if (!boilerplateLine(line, blockComment)) ++workingLines;
        }
    }
    std::cout << "files=" << files << "\n";
    std::cout << "working_loc=" << workingLines << "\n";
    return workingLines >= 30000 ? 0 : 2;
}
