#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "dotenv.h"
#include "WeatherForecast.h"


int main(int argc, const char* argv[]) {
    if (argc == 1) {
        std::cerr << "No configuration file";
        return EXIT_FAILURE;
    }

    std::string file_path = argv[1];

    try {
        dotenv::load_dotenv();
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }

    try {
        WeatherForecast weather_app(file_path);
        weather_app.Start();
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
