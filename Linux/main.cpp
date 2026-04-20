#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <algorithm>

namespace fs = std::filesystem;

const std::string VERSION = "Convertus v1.1.0 (Linux)";

void print_help() {
    std::cout << "Convertus CLI (Linux)\n\n";
    std::cout << "Usage:\n";
    std::cout << "  convertus -C <file1> [file2...] -T <extension> [-O <output_folder>] [-f]\n";
    std::cout << "  convertus info -C <file1> [file2...]\n";
    std::cout << "  convertus -Uninstall\n\n";

    std::cout << "Options:\n";
    std::cout << "  --help        Show this help message\n";
    std::cout << "  --version     Show version\n";
    std::cout << "  -C            Files to convert\n";
    std::cout << "  -T            Target extension\n";
    std::cout << "  -O            Output directory\n";
    std::cout << "  -f            Force overwrite existing files\n";
    std::cout << "  -Uninstall    Remove Convertus\n";
}

void print_version() {
    std::cout << VERSION << '\n';
}

bool convert_file(const fs::path& input, const std::string& ext, const fs::path& output, bool force) {
    if (!fs::exists(input))
        return false;

    fs::path out = output / (input.stem().string() + ext);

    if (fs::exists(out) && !force)
        return false;

    try {
        fs::copy_file(input, out, fs::copy_options::overwrite_existing);
        std::cout << "Saved: " << out << '\n';

        std::string cmd = "xdg-open \"" + out.string() + "\" > /dev/null 2>&1 &";
        std::system(cmd.c_str());

        return true;
    } catch (...) {
        return false;
    }
}

void info_file(const fs::path& file) {
    if (!fs::exists(file))
        return;

    auto size = fs::file_size(file);
    auto ftime = fs::last_write_time(file);

    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );

    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::cout << "File: " << file << '\n';
    std::cout << "Size: " << size << " bytes\n";
    std::cout << "Modified: "
              << std::put_time(std::localtime(&cftime), "%F %T") << "\n\n";
}

void uninstall_convertus() {
    std::cout << "Removing Convertus...\n";

    fs::path exe_path = fs::path(std::getenv("HOME")) / ".local/bin/convertus";

    if (fs::exists(exe_path)) {
        fs::remove(exe_path);
        std::cout << "Removed from ~/.local/bin\n";
    } else {
        std::cout << "Convertus not found in ~/.local/bin\n";
    }

    std::cout << "Done.\n";
}

int levenshtein(const std::string& s1, const std::string& s2) {
    size_t m = s1.size();
    size_t n = s2.size();

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (size_t i = 0; i <= m; i++)
        dp[i][0] = i;

    for (size_t j = 0; j <= n; j++)
        dp[0][j] = j;

    for (size_t i = 1; i <= m; i++) {
        for (size_t j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({
                    dp[i - 1][j],
                    dp[i][j - 1],
                    dp[i - 1][j - 1]
                });
            }
        }
    }

    return dp[m][n];
}

void suggest_command(const std::string& arg) {
    std::vector<std::string> valid_args = {
        "--help",
        "--version",
        "-C",
        "-T",
        "-O",
        "-f",
        "-Uninstall",
        "info"
    };

    std::string best_match;
    int min_dist = 1000;

    for (const auto& v : valid_args) {
        int d = levenshtein(arg, v);

        if (d < min_dist) {
            min_dist = d;
            best_match = v;
        }
    }

    std::cout << "Error: Unknown argument \"" << arg << "\".\n";

    if (min_dist <= 3)
        std::cout << "Did you mean: " << best_match << "?\n";
    else
        std::cout << "Use --help to see valid commands.\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    std::vector<std::string> files;
    std::string target;

    fs::path output = fs::path(std::getenv("HOME")) / "Downloads";

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

            if (!target.empty() && target[0] != '.')
                target = "." + target;
        }
        else if (arg == "-O" && i + 1 < argc) {
            output = fs::absolute(argv[++i]);

            if (!fs::exists(output))
                fs::create_directories(output);
        }
        else {
            suggest_command(arg);
            return 0;
        }
    }

    if (files.empty()) {
        print_help();
        return 0;
    }

    if (info_mode) {
        for (const auto& f : files)
            info_file(fs::absolute(f));

        return 0;
    }

    if (target.empty()) {
        print_help();
        return 0;
    }

    for (const auto& f : files)
        convert_file(fs::absolute(f), target, output, force);

    return 0;
}