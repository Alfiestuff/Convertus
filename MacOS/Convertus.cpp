#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

const std::string VERSION = "Convertus 1.7";

// ---------------- HELP ----------------
void print_help() {
    std::cout << "Convertus CLI\n\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <file1> [file2...] -T <extension>\n";
    std::cout << "  Convertus info -C <file>\n\n";

    std::cout << "Options:\n";
    std::cout << "  --help        Show help\n";
    std::cout << "  --version     Show version\n";
    std::cout << "  -C            Files to convert\n";
    std::cout << "  -T            Target extension\n";
    std::cout << "  -O            Output directory\n";
    std::cout << "  -f            Force overwrite\n";
    std::cout << "  -Uninstall    Remove Convertus\n";
}

// ---------------- VERSION ----------------
void print_version() {
    std::cout << VERSION << std::endl;
}

// ---------------- CONVERT ----------------
bool convert_file(const fs::path& input, const std::string& ext, const fs::path& output, bool force) {

    if (!fs::exists(input))
        return false;

    fs::path out = output / (input.stem().string() + ext);

    if (fs::exists(out) && !force)
        return false;

    try {
        fs::copy_file(input, out, fs::copy_options::overwrite_existing);
        std::cout << "Saved: " << out << "\n";
        return true;
    }
    catch (...) {
        return false;
    }
}

// ---------------- INFO ----------------
void info_file(const fs::path& file) {

    if (!fs::exists(file))
        return;

    auto size = fs::file_size(file);
    auto ftime = fs::last_write_time(file);

    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );

    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::cout << "File: " << file << "\n";
    std::cout << "Size: " << size << " bytes\n";
    std::cout << "Modified: "
              << std::put_time(std::localtime(&cftime), "%F %T") << "\n";
}

// ---------------- UNINSTALL ----------------
void uninstall_convertus() {

    std::cout << "Removing Convertus...\n";

    fs::path bin = "/usr/local/bin/Convertus";

    if (fs::exists(bin))
        fs::remove(bin);

    std::cout << "Convertus removed.\n";
}

// ---------------- MAIN ----------------
int main(int argc, char* argv[]) {

    if (argc == 1) {
        print_help();
        return 0;
    }

    std::vector<std::string> files;
    std::string target;
    fs::path output = fs::path(getenv("HOME")) / "Downloads";

    bool force = false;
    bool info_mode = false;

    for (int i = 1; i < argc; i++) {

        std::string arg = argv[i];

        if (arg == "--help") {
            print_help();
            return 0;
        }

        else if (arg == "--version") {
            print_version();
            return 0;
        }

        else if (arg == "-Uninstall") {
            uninstall_convertus();
            return 0;
        }

        else if (arg == "-f") {
            force = true;
        }

        else if (arg == "info") {
            info_mode = true;
        }

        else if (arg == "-C" && i + 1 < argc) {

            while (i + 1 < argc && argv[i + 1][0] != '-') {
                files.push_back(argv[++i]);
            }

        }

        else if (arg == "-T" && i + 1 < argc) {

            target = argv[++i];

            if (target[0] != '.')
                target = "." + target;

        }

        else if (arg == "-O" && i + 1 < argc) {

            output = fs::absolute(argv[++i]);

            if (!fs::exists(output))
                fs::create_directories(output);

        }

        else {

            // ANY unknown command just shows help
            print_help();
            return 0;
        }
    }

    if (files.empty()) {
        print_help();
        return 0;
    }

    if (info_mode) {

        for (auto& f : files)
            info_file(fs::absolute(f));

        return 0;
    }

    if (target.empty()) {
        print_help();
        return 0;
    }

    for (auto& f : files)
        convert_file(fs::absolute(f), target, output, force);

    return 0;
}
