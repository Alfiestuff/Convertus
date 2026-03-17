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

const std::string VERSION = "Convertus 1.9.4 (macOS)";

void print_help() {
    std::cout << "Convertus CLI (macOS)\n\n";
    std::cout << "Usage:\n";
    std::cout << "  Convertus -C <file1> [file2...] -T <extension> [-O <output_folder>] [-f]\n";
    std::cout << "  Convertus info -C <file1> [file2...]\n";
    std::cout << "  Convertus -U <value> <from_unit> <to_unit>\n";
    std::cout << "  Convertus -Uninstall\n\n";
}

void print_version() {
    std::cout << VERSION << std::endl;
}

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

void uninstall_convertus() {
    std::cout << "Removing Convertus...\n";
    fs::path exe = "/usr/local/bin/Convertus";

    if (fs::exists(exe)) fs::remove(exe);

    std::cout << "Convertus removed.\n";
}

double convert_length(double v, const std::string& f, const std::string& t) {
    std::unordered_map<std::string,double> m = {{"m",1},{"km",1000},{"cm",0.01},{"mm",0.001}};
    return v * m[f] / m[t];
}

double convert_weight(double v, const std::string& f, const std::string& t) {
    std::unordered_map<std::string,double> m = {{"kg",1},{"g",0.001},{"lb",0.453592},{"oz",0.0283495}};
    return v * m[f] / m[t];
}

double convert_temp(double v, const std::string& f, const std::string& t) {
    if(f == t) return v;
    if(f=="C"&&t=="F") return v*9/5+32;
    if(f=="F"&&t=="C") return (v-32)*5/9;
    if(f=="C"&&t=="K") return v+273.15;
    if(f=="K"&&t=="C") return v-273.15;
    if(f=="F"&&t=="K") return (v-32)*5/9+273.15;
    if(f=="K"&&t=="F") return (v-273.15)*9/5+32;
    return NAN;
}

void convert_unit(double v, const std::string& f, const std::string& t) {
    if(f=="C"||f=="F"||f=="K")
        std::cout<<v<<" "<<f<<" = "<<convert_temp(v,f,t)<<" "<<t<<"\n";
    else if(f=="m"||f=="km"||f=="cm"||f=="mm")
        std::cout<<v<<" "<<f<<" = "<<convert_length(v,f,t)<<" "<<t<<"\n";
    else if(f=="kg"||f=="g"||f=="lb"||f=="oz")
        std::cout<<v<<" "<<f<<" = "<<convert_weight(v,f,t)<<" "<<t<<"\n";
    else
        std::cout<<"Unknown units\n";
}

int levenshtein(const std::string& a, const std::string& b) {
    std::vector<std::vector<int>> dp(a.size()+1, std::vector<int>(b.size()+1));
    for(int i=0;i<=a.size();i++) dp[i][0]=i;
    for(int j=0;j<=b.size();j++) dp[0][j]=j;
    for(int i=1;i<=a.size();i++){
        for(int j=1;j<=b.size();j++){
            if(a[i-1]==b[j-1]) dp[i][j]=dp[i-1][j-1];
            else dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});
        }
    }
    return dp[a.size()][b.size()];
}

void suggest(const std::string& arg) {
    std::vector<std::string> valid = {"--help","--version","-C","-T","-O","-f","-Uninstall","info","-U"};
    std::string best;
    int dist=999;

    for(auto &v:valid){
        int d=levenshtein(arg,v);
        if(d<dist){dist=d;best=v;}
    }

    std::cout<<"Error: Unknown argument \""<<arg<<"\"\n";
    if(dist<=3) std::cout<<"Did you mean: "<<best<<"?\n";
}

int main(int argc,char* argv[]) {
    if(argc==1){ print_help(); return 0; }

    std::vector<std::string> files;
    std::string target;

    fs::path output = fs::path(std::getenv("HOME")) / "Downloads";
    bool force=false;
    bool info=false;

    for(int i=1;i<argc;i++){
        std::string arg=argv[i];

        if(arg=="--help"){ print_help(); return 0; }
        else if(arg=="--version"){ print_version(); return 0; }
        else if(arg=="-Uninstall"){ uninstall_convertus(); return 0; }
        else if(arg=="-f"){ force=true; }
        else if(arg=="info"){ info=true; }
        else if(arg=="-C" && i+1<argc){
            while(i+1<argc && argv[i+1][0]!='-') files.push_back(argv[++i]);
        }
        else if(arg=="-T" && i+1<argc){
            target=argv[++i];
            if(target[0]!='.') target="."+target;
        }
        else if(arg=="-O" && i+1<argc){
            output=fs::absolute(argv[++i]);
        }
        else if(arg=="-U" && i+3<argc){
            convert_unit(std::stod(argv[++i]), argv[++i], argv[++i]);
            return 0;
        }
        else {
            suggest(arg);
            return 0;
        }
    }

    if(files.empty()){ print_help(); return 0; }

    if(info){
        for(auto &f:files) info_file(fs::absolute(f));
        return 0;
    }

    if(target.empty()){ print_help(); return 0; }

    for(auto &f:files)
        convert_file(fs::absolute(f),target,output,force);

    return 0;
}