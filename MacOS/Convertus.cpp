#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

const std::string VERSION = "Convertus 1.3";
const std::string GITHUB_REPO = "Alfiestuff/Convertus"; // Replace with your GitHub repo

void print_help() {
    std::cout << "Convertus - Blazing-fast local file converter\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <path_to_file> -T <target_extension>\n";
    std::cout << "  Convertus --upgrade   Upgrade to latest version\n";
    std::cout << "  Convertus --help      Show this help message\n";
    std::cout << "  Convertus --version   Show version\n";
}

void print_version() {
    std::cout << VERSION << std::endl;
}

bool check_internet() {
    return std::system("ping -c 1 github.com > /dev/null 2>&1") == 0;
}

bool convert_file_fast(const fs::path& input_path, const std::string& target_ext) {
    if (!fs::exists(input_path)) {
        std::cerr << "File not found: " << input_path << std::endl;
        return false;
    }

    fs::path filename = input_path.stem();
    fs::path output_path = fs::path(getenv("HOME")) / "Downloads" / (filename.string() + target_ext);

    std::string command = "sips -s format " + target_ext.substr(1) + " " + input_path.string() + " --out " + output_path.string();
    if (std::system(command.c_str()) != 0) {
        std::cerr << "Conversion failed." << std::endl;
        return false;
    }

    std::cout << "Converted file saved to: " << output_path << std::endl;
    return true;
}

bool upgrade_mac() {
    if (!check_internet()) {
        std::cerr << "No internet connection. Cannot upgrade.\n";
        return false;
    }

    std::cout << "Checking latest release...\n";

    std::string api_url = "https://api.github.com/repos/" + GITHUB_REPO + "/releases/latest";
    std::string tmp_json = "/tmp/convertus_release.json";

    std::string curl_cmd = "curl -sL " + api_url + " -o " + tmp_json;
    if (std::system(curl_cmd.c_str()) != 0) {
        std::cerr << "Failed to fetch release info.\n";
        return false;
    }

    std::string pkg_url;
    std::ifstream file(tmp_json);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(".pkg\"") != std::string::npos && line.find("MacOS") != std::string::npos) {
            size_t start = line.find("https://");
            size_t end = line.find("\"", start);
            pkg_url = line.substr(start, end - start);
            break;
        }
    }
    file.close();

    if (pkg_url.empty()) {
        std::cerr << "Could not find macOS PKG in latest release.\n";
        return false;
    }

    std::cout << "Downloading latest package...\n";
    std::string tmp_pkg = "/tmp/Convertus-latest.pkg";
    std::string download_cmd = "curl -L -o " + tmp_pkg + " " + pkg_url;
    if (std::system(download_cmd.c_str()) != 0) {
        std::cerr << "Failed to download package.\n";
        return false;
    }

    std::cout << "Removing old binary...\n";
    std::system("sudo rm -f /usr/local/bin/Convertus");

    std::cout << "Installing new package...\n";
    std::string install_cmd = "sudo installer -pkg " + tmp_pkg + " -target /";
    if (std::system(install_cmd.c_str()) != 0) {
        std::cerr << "Failed to install package.\n";
        return false;
    }

    std::cout << "Upgrade complete! Version is now updated.\n";
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
        } else if (arg == "--upgrade") {
            return upgrade_mac() ? 0 : 1;
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
        return convert_file_fast(file_path, target_ext) ? 0 : 1;
    }

    return 0;
}
