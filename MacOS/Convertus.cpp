#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <cmath>

namespace fs = std::filesystem;

const std::string VERSION = "Convertus 2.0";

// Show usage instructions
void print_help() {
    std::cout << "Convertus CLI\n\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <file1> [file2...] -T <extension> [-O <output_folder>] [-f]\n";
    std::cout << "  Convertus info -C <file1> [file2...]\n";
    std::cout << "  Convertus -U <value> <from_unit> <to_unit>\n";
    std::cout << "  Convertus -Uninstall\n\n";

    std::cout << "Options:\n";
    std::cout << "  --help        Show this help message\n";
    std::cout << "  --version     Show version\n";
    std::cout << "  -C            Files to convert\n";
    std::cout << "  -T            Target extension\n";
    std::cout << "  -O            Output directory\n";
    std::cout << "  -f            Force overwrite existing files\n";
    std::cout << "  -Uninstall    Remove Convertus\n";
    std::cout << "  -U            Unit conversion\n";
}

// Show current version
void print_version() {
    std::cout << VERSION << std::endl;
}

// Copy a file to the target extension
bool convert_file(const fs::path& input, const std::string& ext, const fs::path& output, bool force) {
    if (!fs::exists(input)) return false;

    fs::path out = output / (input.stem().string() + ext);

    if (fs::exists(out) && !force) return false;

    try {
        fs::copy_file(input, out, fs::copy_options::overwrite_existing);
        std::cout << "Saved: " << out << "\n";
        return true;
    } catch (...) {
        return false;
    }
}

// Show file info: size and last modified
void info_file(const fs::path& file) {
    if (!fs::exists(file)) return;

    auto size = fs::file_size(file);
    auto ftime = fs::last_write_time(file);

    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::cout << "File: " << file << "\n";
    std::cout << "Size: " << size << " bytes\n";
    std::cout << "Modified: " << std::put_time(std::localtime(&cftime), "%F %T") << "\n";
}

// Remove Convertus from system
void uninstall_convertus() {
    std::cout << "Removing Convertus...\n";
    fs::path bin = "/usr/local/bin/Convertus";

    if (fs::exists(bin)) fs::remove(bin);

    std::cout << "Convertus removed.\n";
}

// Unit conversion: length, weight, temperature
double convert_length(double value, const std::string& from, const std::string& to) {
    std::unordered_map<std::string,double> factors = {{"m",1},{"km",1000},{"cm",0.01},{"mm",0.001}};
    return value * factors[from] / factors[to];
}

double convert_weight(double value, const std::string& from, const std::string& to) {
    std::unordered_map<std::string,double> factors = {{"kg",1},{"g",0.001},{"lb",0.453592},{"oz",0.0283495}};
    return value * factors[from] / factors[to];
}

double convert_temperature(double value, const std::string& from, const std::string& to) {
    if(from == to) return value;
    if(from == "C" && to == "F") return value*9/5 + 32;
    if(from == "F" && to == "C") return (value-32)*5/9;
    if(from == "C" && to == "K") return value + 273.15;
    if(from == "K" && to == "C") return value - 273.15;
    if(from == "F" && to == "K") return (value-32)*5/9 + 273.15;
    if(from == "K" && to == "F") return (value-273.15)*9/5 + 32;
    return NAN;
}

// Run unit conversion and print result
void convert_unit_command(double value, const std::string& from, const std::string& to) {
    if(from == "C" || from == "F" || from == "K")
        std::cout << value << " " << from << " = " << convert_temperature(value,from,to) << " " << to << "\n";
    else if(from == "m" || from == "km" || from == "cm" || from == "mm")
        std::cout << value << " " << from << " = " << convert_length(value,from,to) << " " << to << "\n";
    else if(from == "kg" || from == "g" || from == "lb" || from == "oz")
        std::cout << value << " " << from << " = " << convert_weight(value,from,to) << " " << to << "\n";
    else
        std::cout << "Unknown unit: " << from << " or " << to << "\n";
}

// Simple Levenshtein distance to find closest valid argument
int levenshtein(const std::string &s1, const std::string &s2) {
    size_t m = s1.size(), n = s2.size();
    std::vector<std::vector<int>> dp(m+1,std::vector<int>(n+1));
    for(size_t i=0;i<=m;i++) dp[i][0]=i;
    for(size_t j=0;j<=n;j++) dp[0][j]=j;
    for(size_t i=1;i<=m;i++){
        for(size_t j=1;j<=n;j++){
            if(s1[i-1]==s2[j-1]) dp[i][j]=dp[i-1][j-1];
            else dp[i][j]=1+std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
        }
    }
    return dp[m][n];
}

// Suggest closest valid argument
void suggest_command(const std::string& arg) {
    std::vector<std::string> valid_args = {"--help","--version","-C","-T","-O","-f","-Uninstall","info","-U"};
    std::string best_match;
    int min_dist = 1000;

    for(auto &v : valid_args){
        int d = levenshtein(arg,v);
        if(d < min_dist){ min_dist=d; best_match=v; }
    }

    std::cout << "Error: Unknown argument \"" << arg << "\".\n";
    if(min_dist <= 3)
        std::cout << "Did you mean: " << best_match << "?\n";
    else
        std::cout << "Use --help to see valid commands.\n";
}

// ---------------- MAIN ----------------
int main(int argc, char* argv[]) {
    if(argc == 1) { print_help(); return 0; }

    std::vector<std::string> files;
    std::string target;
    fs::path output = fs::path(getenv("HOME")) / "Downloads";
    bool force = false;
    bool info_mode = false;

    for(int i=1;i<argc;i++){
        std::string arg = argv[i];

        if(arg == "--help") { print_help(); return 0; }
        else if(arg == "--version") { print_version(); return 0; }
        else if(arg == "-Uninstall") { uninstall_convertus(); return 0; }
        else if(arg == "-f") { force = true; }
        else if(arg == "info") { info_mode = true; }
        else if(arg == "-C" && i+1 < argc) {
            while(i+1 < argc && argv[i+1][0] != '-') files.push_back(argv[++i]);
        }
        else if(arg == "-T" && i+1 < argc) {
            target = argv[++i]; if(target[0] != '.') target = "." + target;
        }
        else if(arg == "-O" && i+1 < argc) {
            output = fs::absolute(argv[++i]);
            if(!fs::exists(output)) fs::create_directories(output);
        }
        else if(arg == "-U" && i+3 < argc) {
            double value = std::stod(argv[++i]);
            std::string from = argv[++i];
            std::string to = argv[++i];
            convert_unit_command(value,from,to);
            return 0;
        }
        else {
            // If unknown argument, suggest closest match
            suggest_command(arg);
            return 0;
        }
    }

    if(files.empty()) { print_help(); return 0; }

    if(info_mode) {
        for(auto &f : files) info_file(fs::absolute(f));
        return 0;
    }

    if(target.empty()) { print_help(); return 0; }

    for(auto &f : files)
        convert_file(fs::absolute(f), target, output, force);

    return 0;
}
