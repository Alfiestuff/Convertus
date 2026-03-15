#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

const std::string VERSION = "Convertus 1.0";

void print_help() {
    std::cout << "Convertus CLI - Local File Converter\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <path_to_file> -T <target_extension>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help       Show this help message\n";
    std::cout << "  --version    Show version\n";
}

void print_version() {
    std::cout << VERSION << std::endl;
}

// Function to convert file (copy with new extension)
bool convert_file(const fs::path& input_path, const std::string& target_ext) {
    if (!fs::exists(input_path)) {
        std::cerr << "Error: File does not exist: " << input_path << std::endl;
        return false;
    }

    // Get filename without extension
    fs::path filename = input_path.stem();

    // Get Downloads folder from HOME env variable
    const char* home = getenv("HOME");
    if (!home) {
        std::cerr << "Error: Cannot find HOME directory." << std::endl;
        return false;
    }
    fs::path downloads = fs::path(home) / "Downloads";

    if (!fs::exists(downloads)) {
        fs::create_directory(downloads);
    }

    // Build output path
    fs::path output_path = downloads / (filename.string() + target_ext);

    try {
        fs::copy_file(input_path, output_path, fs::copy_options::overwrite_existing);
        std::cout << "File converted and saved to: " << output_path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error converting file: " << e.what() << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    std::string input_file;
    std::string target_ext;

    // Simple argument parsing
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

    if (!convert_file(file_path, target_ext)) {
        return 1;
    }

    return 0;
}
