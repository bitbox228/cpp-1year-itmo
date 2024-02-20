#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

class InvalidConfigPathException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Invalid path to the configuration file.";
    }
};

class NoCitiesException : public std::exception {
public:
    const char* what() const noexcept override {
        return "There are no cities in the configuration file.";
    }
};

class WeatherForecast {
private:

    static const uint8_t DEFAULT_UPDATE_RATE = 60;
    static const uint8_t MAX_DAYS_COUNT = 9;
    static const uint8_t MIN_DAYS_COUNT = 1;
    static const uint8_t HOURS_PER_DAY = 24;
    static const uint8_t HOURS_PER_PART_OF_DAY = 6;
    static const uint8_t DAYS_PER_WEEK = 7;
    static const uint8_t MONTHS_PER_YEAR = 12;
    static const uint8_t WIND_DIRECTIONS_COUNT = 8;
    static const uint8_t PARTS_OF_DAY_PER_DAY = HOURS_PER_DAY / HOURS_PER_PART_OF_DAY;

    static const std::array<std::string, PARTS_OF_DAY_PER_DAY> DAY_PART_STRING;
    static const std::array<std::string, DAYS_PER_WEEK> WEEK_DAY_STRING;
    static const std::array<std::string, MONTHS_PER_YEAR> MONTH_STRING;
    static const std::unordered_map<uint8_t, std::string> WEATHERCODE_STRING;
    static const std::array<std::string, WIND_DIRECTIONS_COUNT> WIND_DIRECTION;

    static const std::string WEATHER_URL;
    static const std::string CITY_URL;
    static const std::string API_KEY_ENV_NAME;

    static const cpr::Parameters PARAMETERS;

    enum Status {
        NothingParsed = 0,
        CityCoordinatesParsed,
        WeatherForecastParsed,
        NotACity
    };

    struct CityForecast {
        nlohmann::json json;
        Status status;

        CityForecast() : json(), status(Status::NothingParsed) {}
    };

    std::map<std::string, CityForecast>::iterator current_city_;
    std::map<std::string, CityForecast>::iterator end_city_;
    std::map<std::string, CityForecast> cities_;

    size_t update_rate_;
    uint8_t days_count_;

    ftxui::Elements forecast_;

    void UpdateData() noexcept;

    void DeleteNotACities() noexcept;

    std::string ParseDate(uint8_t day_number) const noexcept;

    static ftxui::Element ForecastPicture(uint8_t WMO_code) noexcept;

    ftxui::Element PartsOfDayForecast(uint8_t day_number) const noexcept;

    ftxui::Element CurrentForecast() const noexcept;

    void GenerateForecast() noexcept;

    bool IncreaseDays() noexcept;

    bool DecreaseDays() noexcept;

    bool IncreaseCity() noexcept;

    bool DecreaseCity() noexcept;

public:

    WeatherForecast() noexcept : update_rate_(DEFAULT_UPDATE_RATE), days_count_(MIN_DAYS_COUNT) {}

    WeatherForecast(const std::string& file_name);

    void Start() noexcept;
};