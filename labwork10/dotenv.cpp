#include "dotenv.h"

const std::string dotenv::prefix = "WEATHER_WIDGET";
const std::string dotenv::name = "API_KEY";
const std::filesystem::path dotenv::filename = ".env";

void dotenv::load_dotenv() {
    std::string envvar_key = prefix + "_" + name;
    if (std::getenv(envvar_key.c_str()) != nullptr) {
        return;
    }
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw NoApiKeyException();
    }
    std::string line;
    while (std::getline(file, line)) {
        std::string key, value;
        std::istringstream ss(line);
        std::getline(ss, key, '=');
        std::getline(ss, value);
        if (key == name) {
            line = prefix + "_" + line;
            putenv(line.c_str());
            return;
        }
    }
    throw NoApiKeyException();
}
