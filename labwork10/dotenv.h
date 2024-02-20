#include <stdlib.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <iostream>

class NoApiKeyException : public std::exception {
public:
    const char* what() const noexcept override {
        return "There is no API-key in the configuration file.";
    }
};

class dotenv {

    const static std::string prefix;
    const static std::string name;
    const static std::filesystem::path filename;
public:

    static void load_dotenv();
};