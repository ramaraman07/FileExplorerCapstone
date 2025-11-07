// file_explorer.cpp
// Console-Based File Explorer Application using C++ (C++17)
// Works on Windows, macOS, and Linux using <filesystem>
// Author: Rama Raman Sarangi
// Date: November 2025

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

std::string perms_to_string(fs::perms p) {
    auto check = [&](fs::perms bit, char c){ return ((p & bit) != fs::perms::none) ? c : '-'; };
    std::string s;
    s += check(fs::perms::owner_read, 'r');
    s += check(fs::perms::owner_write, 'w');
    s += check(fs::perms::owner_exec, 'x');
    s += check(fs::perms::group_read, 'r');
    s += check(fs::perms::group_write, 'w');
    s += check(fs::perms::group_exec, 'x');
    s += check(fs::perms::others_read, 'r');
    s += check(fs::perms::others_write, 'w');
    s += check(fs::perms::others_exec, 'x');
    return s;
}

std::string time_to_string(fs::file_time_type ftime) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(ftime - fs::file_time_type::clock::now()
                                                        + system_clock::now());
    std::time_t tt = system_clock::to_time_t(sctp);
    char buf[64];
#ifdef _WIN32
    ctime_s(buf, sizeof(buf), &tt);
#else
    ctime_r(&tt, buf);
#endif
    std::string res(buf);
    if (!res.empty() && res.back() == '\n') res.pop_back();
    return res;
}

void list_dir(const fs::path& cur) {
    std::cout << "\nCurrent Directory: " << cur.string() << "\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << std::left << std::setw(8) << "TYPE"
              << std::setw(12) << "PERMS"
              << std::setw(12) << "SIZE(B)"
              << std::setw(24) << "MODIFIED"
              << "NAME\n";
    std::cout << "------------------------------------------------------------\n";
    try {
        for (const auto& entry : fs::directory_iterator(cur)) {
            const auto path = entry.path();
            auto type = entry.is_directory() ? "[DIR]" : (entry.is_symlink() ? "[LNK]" : "[FILE]");
            std::string perm = "----------";
            std::uintmax_t size = 0;
            std::string mod = "";
            try {
                auto st = fs::status(path);
                perm = perms_to_string(st.permissions());
            } catch(...) {}
            if (entry.is_regular_file()) {
                try { size = entry.file_size(); } catch(...) {}
            }
            try { mod = time_to_string(fs::last_write_time(path)); } catch(...) {}
            std::cout << std::left << std::setw(8) << type
                      << std::setw(12) << perm
                      << std::setw(12) << size
                      << std::setw(24) << mod
                      << path.filename().string() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error listing directory: " << e.what() << "\n";
    }
}

bool input_path(const std::string& prompt, fs::path& out) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    if (s.empty()) return false;
    out = fs::path(s);
    return true;
}

void create_file(const fs::path& p) {
    try {
        if (fs::exists(p)) { std::cout << "Path already exists.\n"; return; }
        std::ofstream ofs(p.string());
        if (!ofs) { std::cout << "Failed to create file.\n"; return; }
        ofs << ""; ofs.close();
        std::cout << "File created: " << p << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void create_directory_path(const fs::path& p) {
    try {
        if (fs::exists(p)) { std::cout << "Path already exists.\n"; return; }
        fs::create_directories(p);
        std::cout << "Directory created: " << p << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void delete_path(const fs::path& p) {
    try {
        if (!fs::exists(p)) { std::cout << "Path does not exist.\n"; return; }
        std::uintmax_t count = fs::is_directory(p) ? fs::remove_all(p) : (fs::remove(p) ? 1 : 0);
        std::cout << "Deleted entries: " << count << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void copy_path(const fs::path& from, const fs::path& to) {
    try {
        if (!fs::exists(from)) { std::cout << "Source does not exist.\n"; return; }
        if (fs::is_directory(from)) {
            fs::copy(from, to, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        } else {
            fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        }
        std::cout << "Copied to: " << to << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void move_path(const fs::path& from, const fs::path& to) {
    try {
        fs::rename(from, to);
        std::cout << "Moved/Renamed to: " << to << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void search_recursive(const fs::path& root, const std::string& needle) {
    try {
        for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied);
             it != fs::recursive_directory_iterator(); ++it) {
            const auto& p = it->path();
            if (p.filename().string().find(needle) != std::string::npos) {
                std::cout << p.string() << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching: " << e.what() << "\n";
    }
}

void print_menu() {
    std::cout << "\nCommands:\n"
              << "1. List current directory\n"
              << "2. Enter directory\n"
              << "3. Go up (..)\n"
              << "4. Create file\n"
              << "5. Create directory\n"
              << "6. Delete file/directory\n"
              << "7. Copy file/directory\n"
              << "8. Move/Rename file/directory\n"
              << "9. Search by name (recursive)\n"
              << "0. Exit\n"
              << "Choose: ";
}

int main() {
#ifdef _WIN32
    // Enable colored console output on Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    fs::path cur = fs::current_path();
    std::string choice;
    while (true) {
        list_dir(cur);
        print_menu();
        if (!std::getline(std::cin, choice)) break;
        if (choice == "1") {
            continue; // already listed
        } else if (choice == "2") {
            fs::path dir;
            if (input_path("Enter directory name: ", dir)) {
                fs::path cand = dir.is_absolute() ? dir : (cur / dir);
                if (fs::exists(cand) && fs::is_directory(cand)) cur = fs::canonical(cand);
                else std::cout << "Not a directory.\n";
            }
        } else if (choice == "3") {
            if (cur.has_parent_path()) cur = cur.parent_path();
        } else if (choice == "4") {
            fs::path p;
            if (input_path("Enter file path to create: ", p)) {
                p = p.is_absolute() ? p : (cur / p);
                create_file(p);
            }
        } else if (choice == "5") {
            fs::path p;
            if (input_path("Enter directory path to create: ", p)) {
                p = p.is_absolute() ? p : (cur / p);
                create_directory_path(p);
            }
        } else if (choice == "6") {
            fs::path p;
            if (input_path("Enter file/directory to delete: ", p)) {
                p = p.is_absolute() ? p : (cur / p);
                delete_path(p);
            }
        } else if (choice == "7") {
            fs::path src, dst;
            if (input_path("Enter source path: ", src) && input_path("Enter destination path: ", dst)) {
                src = src.is_absolute() ? src : (cur / src);
                dst = dst.is_absolute() ? dst : (cur / dst);
                copy_path(src, dst);
            }
        } else if (choice == "8") {
            fs::path src, dst;
            if (input_path("Enter source path: ", src) && input_path("Enter destination path: ", dst)) {
                src = src.is_absolute() ? src : (cur / src);
                dst = dst.is_absolute() ? dst : (cur / dst);
                move_path(src, dst);
            }
        } else if (choice == "9") {
            std::cout << "Enter name to search: ";
            std::string needle;
            std::getline(std::cin, needle);
            if (!needle.empty()) search_recursive(cur, needle);
        } else if (choice == "0") {
            std::cout << "Goodbye!\n";
            break;
        } else {
            std::cout << "Invalid choice.\n";
        }
    }
    return 0;
}
