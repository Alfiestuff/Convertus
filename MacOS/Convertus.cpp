#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib> // for std::system

namespace fs = std::filesystem;

const std::string VERSION = "Convertus 1.1";

void print_help() {
    std::cout << "Convertus - Blazing-fast local file converter\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <path_to_file> -T <target_extension>\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --help       Show this help message\n";
    std::cout << "  --version    Show version\n";
}

void print_version() {
    std::cout << VERSION << std::endl;
}

bool convert_file_fast(const fs::path& input_path, const std::string& target_ext) {
    if (!fs::exists(input_path)) {
        std::cerr << "File not found: " << input_path << std::endl;
        return false;
    }

    fs::path filename = input_path.stem();
    fs::path output_path = fs::path(getenv("HOME")) / "Downloads" / (filename.string() + target_ext);

    // Use macOS sips command for fast conversion
    std::string command = "sips -s format " + target_ext.substr(1) + " " + input_path.string() + " --out " + output_path.string();
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Conversion failed." << std::endl;
        return false;
    }

    std::cout << "Converted file saved to: " << output_path << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    std::string input_file;
    std::string target_ext;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            print_help();
            return 0;
        } else if (arg == "--version") {
            print_version();
            return 0;
        } else if (arg == "-C" && i + 1 < argc) {
            input_file = argv[++i];
        } else if (arg == "-T" && i + 1 < argc) {
            target_ext = argv[++i];
            if (target_ext[0] != '.') {
                target_ext = "." + target_ext;
            }
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_help();
            return 1;
        }
    }

    if (input_file.empty() || target_ext.empty()) {
        std::cerr << "Error: Missing required arguments.\n";
        print_help();
        return 1;
    }

    fs::path file_path = fs::absolute(input_file);

    if (!convert_file_fast(file_path, target_ext)) {
        return 1;
    }

    return 0;
}
