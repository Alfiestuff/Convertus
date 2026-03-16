#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

const std::string VERSION = "Convertus 1.4";

void print_help() {
    std::cout << "Convertus - Lightweight CLI file converter\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <path_to_file> -T <target_extension>\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --help      Show this help message\n";
    std::cout << "  --version   Show version\n";
    std::cout << "\nSupported:\n";
    std::cout << "  Images: PNG, JPG, JPEG, BMP, GIF, TIFF (via macOS sips)\n";
    std::cout << "  Text/other files: simple copy & rename\n";
}

void print_version() {
    std::cout << VERSION << std::endl;
}

bool is_image(const std::string& ext) {
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif" || ext == ".tiff";
}

bool convert_file(const fs::path& input_path, const std::string& target_ext) {
    if (!fs::exists(input_path)) {
        std::cerr << "File not found: " << input_path << std::endl;
        return false;
    }

    fs::path filename = input_path.stem();
    fs::path output_path = fs::path(getenv("HOME")) / "Downloads" / (filename.string() + target_ext);
    std::string ext = input_path.extension().string();

    int result = -1;

    if (is_image(ext) && is_image(target_ext)) {
        // Use macOS sips for image conversion
        std::string cmd = "sips -s format " + target_ext.substr(1) + " \"" + input_path.string() + "\" --out \"" + output_path.string() + "\"";
        result = std::system(cmd.c_str());
    } else {
        // For text/other files: copy & rename
        try {
            fs::copy_file(input_path, output_path, fs::copy_options::overwrite_existing);
            result = 0;
        } catch (...) {
            result = 1;
        }
    }

    if (result != 0) {
        std::cerr << "Conversion failed.\n";
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

    if (!input_file.empty() && !target_ext.empty()) {
        fs::path file_path = fs::absolute(input_file);
        return convert_file(file_path, target_ext) ? 0 : 1;
    }

    return 0;
}
