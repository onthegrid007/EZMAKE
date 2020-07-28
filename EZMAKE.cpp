#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sstream>

using namespace std;

bool dirExists(const string dirPath) {
    struct stat info;
    if(stat(dirPath.c_str(), &info) != 0)
        return false;
    else if(info.st_mode & S_IFDIR)
        return true;
    return false;
}

bool fileExists(const string dirPath) {
    struct stat info;
    if(stat(dirPath.c_str(), &info) != 0)
        return false;
    else if(info.st_mode & S_IFDIR)
        return false;
    return true;
}

class Config {
    public:
        Config() {}

        Config(const string filePath) :
            m_filePath(filePath),
            m_file(m_filePath) {
            fileCheck();
            if(m_active) parse();
        }

        void parse(const string filePath) {
            m_filePath = filePath;
            if(m_file.is_open())
                m_file.close();
            m_file.open(m_filePath);
            fileCheck();
            if(m_active) parse();
        }

        void parse() {
            if(!m_active) {
                cout << "Can not parse, file is not active!\n";
                return;
            }
            string line;
            while(getline(m_file, line)) {
                istringstream is_line(line);
                string key;
                if(getline(is_line, key, '=')) {
                    string value;
                    if(getline(is_line, value)) 
                        if(value != "")
                            m_dataMap[key] = value;
                }
            }

        }

        bool is_active() const {
            return m_active;
        }

        string find_string(string key, bool silent = false) {
            auto itt = m_dataMap.find(key);
            if(itt != m_dataMap.end())
                return itt->second;
            if(!silent) cout << "Could not find string: " << key << endl;
            return "";
        }

        bool find_bool(string key, bool silent = false) {
            auto itt = m_dataMap.find(key);
            if(itt != m_dataMap.end())
                return ((itt->second == "true") | (itt->second == "True") | (itt->second == "TRUE") | (itt->second == "1"));
            if(!silent) cout << "Could not find bool: " << key << endl;
            return false;
        }

    private:

        void fileCheck() {
            if(!m_file.is_open()) {
                cout << "Config File Not Found At: " << m_filePath << endl;
                return;
            }
            m_active = true;
        }

        string m_filePath;
        ifstream m_file;
        bool m_active = false;
        unordered_map<string, string> m_dataMap;
};

class ArrayFile {
    public:
        ArrayFile() {}

        ArrayFile(const string filePath) :
            m_filePath(filePath),
            m_file(m_filePath) {
            fileCheck();
            if(m_active) parse();
        }

        void parse(const string filePath) {
            m_filePath = filePath;
            if(m_file.is_open())
                m_file.close();
            m_file.open(m_filePath);
            fileCheck();
            if(m_active) parse();
        }

        void parse() {
            if(!m_active) {
                cout << "Can not parse, file is not active!\n";
                return;
            }
            m_data.clear();
            string line;
            while(getline(m_file, line)) {
                if(line != "") m_data.emplace_back(line);
            }
        }

        bool is_active() const {
            return m_active;
        }

        vector<string>& getArray() {
            return m_data;
        }

    private:

        void fileCheck() {
            if(!m_file.is_open()) {
                cout << "Array File Not Found At: " << m_filePath << endl;
                return;
            }
            m_active = true;
        }

        string m_filePath;
        ifstream m_file;
        bool m_active = false;
        vector<string> m_data;
};

Config config;
ArrayFile dependencies_vector;
ArrayFile include_dirs_vector;
ArrayFile lib_dirs_vector;
ArrayFile libs_vector;

bool checkConstantConfigVariables(const string cwd) {
    bool rtn = true;
    string syscall;
    if(
        (config.find_string("PROGRAM_NAME") == "") | 
        (config.find_string("MAIN") == "") | 
        (config.find_string("CXX_COMPILER") == "") | 
        (config.find_string("C_COMPILER") == "")
    ) {
        cout << "Missing required variables in the config file!\n";
        rtn = false;
    }
    else {
        if(!fileExists(cwd + config.find_string("MAIN"))) {
            cout << "Main file does not exist: " << cwd + config.find_string("MAIN") << endl;
            rtn = false;
        }
        // syscall = config.find_string("CXX_COMPILER") + " -v";
        // if(system(syscall.c_str()) != 0) {
        //     cout << "CXX_COMPILER file does not exist: " << config.find_string("CXX_COMPILER") << endl;
        //     rtn = false;
        // }
        // syscall = config.find_string("C_COMPILER") + " -v";
        // if(system(syscall.c_str()) != 0) {
        //     cout << "C_COMPILER file does not exist: " << config.find_string("C_COMPILER") << endl;
        //     rtn = false;
        // }
    }
    return rtn;
}

int main(int argc, char* argv[]) {
    string cwd = "."; // Current working directory path
    string syscall; // Final compiled system call
    string bin; // Current binary directory path
    if(argc > 1)
        cwd = argv[1]; // First argument after EZMAKE is the CWD
    if(cwd.substr(cwd.size() - 1, cwd.size()) != "/")
        cwd = cwd + "/"; // Make sure the path contains a '/' at the end
    config.parse(cwd + "config.ezmake"); // Open the config file and setup map
    if(!config.is_active())
        return -1; // If opening failed, kill the program
    if(!checkConstantConfigVariables(cwd))
        return -1; // If a reqired variables do not exist, kill the program
    bin = cwd + "bin"; // Initilize the binary dir path
    if(!dirExists(bin.c_str())) {
        syscall = "mkdir " + bin;
        system(syscall.c_str()); // Make the binary dir if not exist
    }

    dependencies_vector.parse(cwd + "depends.ezmake");
    if(!dependencies_vector.is_active()) {
        syscall = "echo >> " + cwd + "depends.ezmake";
        system(syscall.c_str()); // Make the depends file if not exist
        dependencies_vector.parse(cwd + "depends.ezmake");
    }

    include_dirs_vector.parse(cwd + "include_dirs.ezmake");
    if(!include_dirs_vector.is_active()) {
        syscall = "echo >> " + cwd + "include_dirs.ezmake";
        system(syscall.c_str()); // Make the include_dirs file if not exist
        include_dirs_vector.parse(cwd + "include_dirs.ezmake");
    }

    lib_dirs_vector.parse(cwd + "lib_dirs.ezmake");
    if(!lib_dirs_vector.is_active()) {
        syscall = "echo >> " + cwd + "lib_dirs.ezmake";
        system(syscall.c_str()); // Make the lib_dirs file if not exist
        lib_dirs_vector.parse(cwd + "lib_dirs.ezmake");
    }

    libs_vector.parse(cwd + "libs.ezmake");
    if(!libs_vector.is_active()) {
        syscall = "echo >> " + cwd + "libs.ezmake";
        system(syscall.c_str()); // Make the libs file if not exist
        libs_vector.parse(cwd + "libs.ezmake");
    }

    if(config.find_string("MAIN").substr(config.find_string("MAIN").size() - 1, config.find_string("MAIN").size()) == "c") {
        syscall = config.find_string("C_COMPILER") + " " + config.find_string("EXTRA_C_ARGS", true) + " "; // Main file is C
    }
    else {
        syscall = config.find_string("CXX_COMPILER") + " " + config.find_string("EXTRA_CXX_ARGS", true) + ((config.find_string("EXTRA_CXX_ARGS", true) != "") ? " " : ""); // Main file is C++
    }
    syscall = syscall + cwd + config.find_string("MAIN") + " "; // Main File

    // Handle other dependencies
    for(unsigned int i = 0; i < dependencies_vector.getArray().size(); i++) {
        syscall = syscall + cwd + dependencies_vector.getArray()[i] + " ";
    }

    // Handle include dirs
    for(int i = 0; i < include_dirs_vector.getArray().size(); i++) {
        syscall = syscall + "-I" + include_dirs_vector.getArray()[i] + " ";
    }

    // Handle lib dirs
    for(int i = 0; i < lib_dirs_vector.getArray().size(); i++) {
        syscall = syscall + "-L" + lib_dirs_vector.getArray()[i] + " ";
    }

    // Handle libs
    for(int i = 0; i < libs_vector.getArray().size(); i++) {
        syscall = syscall + "-l" + libs_vector.getArray()[i] + " ";
    }

    syscall = syscall + "-o " + bin + "/" + config.find_string("PROGRAM_NAME");

    cout << syscall << endl;
    system(syscall.c_str());
    return 0;
}