#include "WeatherForecast.h"

const std::string WeatherForecast::WEATHER_URL = "https://api.open-meteo.com/v1/forecast";
const std::string WeatherForecast::CITY_URL = "https://api.api-ninjas.com/v1/city";
const std::string WeatherForecast::API_KEY_ENV_NAME = "WEATHER_WIDGET_API_KEY";

const cpr::Parameters WeatherForecast::PARAMETERS = {{"timezone",        "auto"},
                                                     {"forecast_days",   std::to_string(MAX_DAYS_COUNT + 1)},
                                                     {"hourly",          "temperature_2m,visibility,weathercode,relativehumidity_2m,apparent_temperature,surface_pressure,windspeed_10m,winddirection_10m,precipitation,precipitation_probability"},
                                                     {"current_weather", "true"}};

const std::array<std::string, WeatherForecast::PARTS_OF_DAY_PER_DAY> WeatherForecast::DAY_PART_STRING = {"Утро", "День",
                                                                                                         "Вечер",
                                                                                                         "Ночь"};
const std::array<std::string, WeatherForecast::DAYS_PER_WEEK> WeatherForecast::WEEK_DAY_STRING = {"Вс.", "Пн.", "Вт.",
                                                                                                  "Ср.", "Чт.", "Пт.",
                                                                                                  "Сб."};
const std::array<std::string, WeatherForecast::MONTHS_PER_YEAR> WeatherForecast::MONTH_STRING = {"янв. ", "февр.",
                                                                                                 "март ", "апр. ",
                                                                                                 "май  ", "июнь ",
                                                                                                 "июль ",
                                                                                                 "авг. ",
                                                                                                 "сент.", "окт. ",
                                                                                                 "нояб.", "дек. "};
const std::unordered_map<uint8_t, std::string> WeatherForecast::WEATHERCODE_STRING = {{0,  "Ясно           "},
                                                                                      {1,  "В основном ясно"},
                                                                                      {2,  "Переменная обл…"},
                                                                                      {3,  "Пасмурно       "},
                                                                                      {45, "Туман          "},
                                                                                      {48, "Изморози       "},
                                                                                      {51, "Морось, слабая "},
                                                                                      {53, "Морось, умерен…"},
                                                                                      {55, "Морось, сильная"},
                                                                                      {56, "Морось со снег…"},
                                                                                      {57, "Морось со снег…"},
                                                                                      {61, "Дождь, слабый  "},
                                                                                      {63, "Дождь, умерен… "},
                                                                                      {65, "Дождь, сильный "},
                                                                                      {66, "Дождь со снегом"},
                                                                                      {67, "Дождь со снегом"},
                                                                                      {71, "Снег, слабый   "},
                                                                                      {73, "Снег, умеренный"},
                                                                                      {75, "Снег, сильный  "},
                                                                                      {77, "Снежные зёрна  "},
                                                                                      {80, "Ливень, слабый "},
                                                                                      {81, "Ливень, умерен…"},
                                                                                      {82, "Ливень, сильный"},
                                                                                      {85, "Ливневый снег  "},
                                                                                      {86, "Ливневый снег  "},
                                                                                      {95, "Гроза          "},
                                                                                      {96, "Гроза и град   "},
                                                                                      {99, "Гроза и град   "}};
const std::array<std::string, WeatherForecast::WIND_DIRECTIONS_COUNT> WeatherForecast::WIND_DIRECTION = {"↓", "↙", "←",
                                                                                                         "↖", "↑", "↗",
                                                                                                         "→", "↘"};


void WeatherForecast::UpdateData() noexcept {
    std::vector<cpr::AsyncResponse> responses;
    for (auto it = cities_.begin(); it != cities_.end(); it++) {
        if (it->second.status == NothingParsed) {
            responses.emplace_back(cpr::GetAsync(cpr::Url{CITY_URL},
                                                 cpr::Header{{"X-Api-Key", std::getenv(API_KEY_ENV_NAME.c_str())}},
                                                 cpr::Parameters{{"name", it->first}}));
        } else if (it->second.status == CityCoordinatesParsed || it->second.status == WeatherForecastParsed) {
            auto params = PARAMETERS;
            params.Add({{"latitude",  to_string(it->second.json["latitude"])},
                        {"longitude", to_string(it->second.json["longitude"])}});
            responses.emplace_back(cpr::GetAsync(cpr::Url{WEATHER_URL}, params));
        }
    }
    size_t i = 0;
    std::vector<std::string> cities_to_delete;
    for (auto it = cities_.begin(); it != cities_.end(); it++) {
        if (it->second.status == NotACity) {
            cities_to_delete.push_back(it->first);
            continue;
        }
        responses[i].wait();
        auto response = responses[i].get();
        if (it->second.status == NothingParsed) {
            if (response.status_code == 200) {
                auto response_json = response.text;
                std::reverse(response_json.begin(), response_json.end());
                response_json.pop_back();
                std::reverse(response_json.begin(), response_json.end());
                response_json.pop_back();
                if (response_json.empty()) {
                    it->second.status = NotACity;
                    cities_to_delete.push_back(it->first);
                } else {
                    it->second.json = nlohmann::json::parse(response_json);
                    it->second.status = CityCoordinatesParsed;
                    auto params = PARAMETERS;
                    params.Add({{"latitude",  to_string(it->second.json["latitude"])},
                                {"longitude", to_string(it->second.json["longitude"])}});
                    responses.emplace_back(cpr::GetAsync(cpr::Url{WEATHER_URL}, params));
                }
            }
        } else if (it->second.status == CityCoordinatesParsed || it->second.status == WeatherForecastParsed) {
            if (response.status_code == 200) {
                auto response_json = response.text;
                it->second.json = nlohmann::json::parse(response_json);
                it->second.status = WeatherForecastParsed;
            } else if (it->second.status == CityCoordinatesParsed) {
                auto params = PARAMETERS;
                params.Add({{"latitude",  to_string(it->second.json["latitude"])},
                            {"longitude", to_string(it->second.json["longitude"])}});
                responses.emplace_back(cpr::GetAsync(cpr::Url{WEATHER_URL}, params));
            }
        }
        i++;
    }

    for (auto it = cities_.begin(); it != cities_.end(); it++) {
        if (it->second.status == CityCoordinatesParsed) {
            responses[i].wait();
            auto response = responses[i].get();
            if (response.status_code == 200) {
                auto response_json = response.text;
                it->second.json = nlohmann::json::parse(response_json);
                it->second.status = WeatherForecastParsed;
            }
            i++;
        }
    }

    auto current_city_name = current_city_->first;
    for (size_t j = 0; j < cities_to_delete.size(); j++) {
        cities_.erase(cities_to_delete[j]);
    }
    current_city_ = cities_.find(current_city_name);
    if (current_city_ == cities_.end()) {
        current_city_ = cities_.begin();
    }
    end_city_ = cities_.begin();
    for (size_t j = 0; j < cities_.size() - 1; j++) {
        end_city_++;
    }
}

std::string WeatherForecast::ParseDate(uint8_t day_number) const noexcept {
    std::string iso_date = current_city_->second.json["hourly"]["time"][day_number * HOURS_PER_DAY];
    std::tm ti = {};
    std::istringstream ss(iso_date);
    ss >> std::get_time(&ti, "%Y-%m-%dT");

    auto time = std::mktime(&ti);

    auto week_day = std::localtime(&time)->tm_wday;
    auto day = std::localtime(&time)->tm_mday;
    auto month = std::localtime(&time)->tm_mon;

    std::string result = WEEK_DAY_STRING[week_day] + " " + ((day < 10) ? "0" : "") + std::to_string(day) + " " +
                         MONTH_STRING[month];
    return result;
}

ftxui::Element WeatherForecast::ForecastPicture(uint8_t WMO_code) noexcept {
    using namespace ftxui;
    Elements result;
    if (WMO_code == 0) {
        result.push_back(color(Color::Palette16::YellowLight, text("    \\   /    ")));
        result.push_back(color(Color::Palette16::YellowLight, text("     .-.     ")));
        result.push_back(color(Color::Palette16::YellowLight, text("  ― (   ) ―  ")));
        result.push_back(color(Color::Palette16::YellowLight, text("     `-’     ")));
        result.push_back(color(Color::Palette16::YellowLight, text("    /   \\    ")));
    } else if (WMO_code == 1) {
        result.push_back(color(Color::Palette16::YellowLight, text("   \\  /      ")));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" _ /\"\"")),
                              color(Color::Palette16::White, text(".-.    "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   \\_")),
                              color(Color::Palette16::White, text("(   ).  "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   /")),
                              color(Color::Palette16::White, text("(___(__) "))));
        result.push_back(text("             "));
    } else if (WMO_code == 2) {
        result.push_back(color(Color::Palette16::YellowLight, text("   \\  /      ")));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" _ /\"")),
                              color(Color::Palette16::White, text(".-.     "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   \\")),
                              color(Color::Palette16::White, text("(   ).   "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   ")),
                              color(Color::Palette16::White, text("(___(__)  "))));
        result.push_back(text("             "));
    } else if (WMO_code == 3) {
        result.push_back(text("             "));
        result.push_back(color(Color::Palette16::White, text("     .--.    ")));
        result.push_back(color(Color::Palette16::White, text("  .-(    ).  ")));
        result.push_back(color(Color::Palette16::White, text(" (___.__)__) ")));
        result.push_back(text("             "));
    } else if (WMO_code == 45) {
        result.push_back(text("             "));
        result.push_back(color(Color::Palette16::White, text(" _ - _ - _ - ")));
        result.push_back(color(Color::Palette16::White, text(" _ - _ - _ - ")));
        result.push_back(color(Color::Palette16::White, text(" _ - _ - _ - ")));
        result.push_back(text("             "));
    } else if (WMO_code == 48) {
        result.push_back(text("             "));
        result.push_back(
                hbox(color(Color::Palette16::White, text(" _ ")), color(Color::Palette16::BlueLight, text("*")),
                     color(Color::Palette16::White, text(" _ - _ ")),
                     color(Color::Palette16::BlueLight, text("* "))));
        result.push_back(hbox(color(Color::Palette16::White, text(" _ - ")),
                              color(Color::Palette16::BlueLight, text("*")),
                              color(Color::Palette16::White, text(" - _ - "))));
        result.push_back(
                hbox(color(Color::Palette16::White, text(" ")), color(Color::Palette16::BlueLight, text("*")),
                     color(Color::Palette16::White, text(" - _ - ")),
                     color(Color::Palette16::BlueLight, text("*")),
                     color(Color::Palette16::White, text(" - "))));
        result.push_back(text("             "));
    } else if (WMO_code == 61 || WMO_code == 63 || WMO_code == 51 || WMO_code == 53 || WMO_code == 55) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(color(Color::Palette16::BlueLight, text("    ‘ ‘ ‘ ‘  ")));
        result.push_back(color(Color::Palette16::BlueLight, text("   ‘ ‘ ‘ ‘   ")));
    } else if (WMO_code == 65) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(color(Color::Palette16::BlueLight, text("  ‚‘‚‘‚‘‚‘   ")));
        result.push_back(color(Color::Palette16::BlueLight, text("  ‚’‚’‚’‚’   ")));
    } else if (WMO_code == 66 || WMO_code == 67 || WMO_code == 56 || WMO_code == 57) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(hbox(color(Color::Palette16::BlueLight, text("    ‘ ")),
                              color(Color::Palette16::White, text("*")),
                              color(Color::Palette16::BlueLight, text(" ‘ ")),
                              color(Color::Palette16::White, text("*  "))));
        result.push_back(hbox(color(Color::Palette16::White, text("   *")),
                              color(Color::Palette16::BlueLight, text(" ‘ ")),
                              color(Color::Palette16::White, text("*")),
                              color(Color::Palette16::BlueLight, text(" ‘   "))));
    } else if (WMO_code == 71 || WMO_code == 73) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(color(Color::Palette16::White, text("    *  *  *  ")));
        result.push_back(color(Color::Palette16::White, text("   *  *  *   ")));
    } else if (WMO_code == 75) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(color(Color::Palette16::White, text("   * * * *   ")));
        result.push_back(color(Color::Palette16::White, text("  * * * *    ")));
    } else if (WMO_code == 77) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(color(Color::Palette16::White, text("   ◦ ◦ ◦ ◦   ")));
        result.push_back(color(Color::Palette16::White, text("  ◦ ◦ ◦ ◦    ")));
    } else if (WMO_code == 80 || WMO_code == 81) {
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" _`/\"\"")),
                              color(Color::Palette16::White, text(".-.    "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("  ,\\_")),
                              color(Color::Palette16::White, text("(   ).  "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   /")),
                              color(Color::Palette16::White, text("(___(__) "))));
        result.push_back(color(Color::Palette16::BlueLight, text("     ‘ ‘ ‘ ‘ ")));
        result.push_back(color(Color::Palette16::BlueLight, text("    ‘ ‘ ‘ ‘  ")));
    } else if (WMO_code == 82) {
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" _`/\"\"")),
                              color(Color::Palette16::White, text(".-.    "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("  ,\\_")),
                              color(Color::Palette16::White, text("(   ).  "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   /")),
                              color(Color::Palette16::White, text("(___(__) "))));
        result.push_back(color(Color::Palette16::BlueLight, text("   ‚‘‚‘‚‘‚‘  ")));
        result.push_back(color(Color::Palette16::BlueLight, text("   ‚’‚’‚’‚’  ")));
    } else if (WMO_code == 85) {
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" _`/\"\"")),
                              color(Color::Palette16::White, text(".-.    "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("  ,\\_")),
                              color(Color::Palette16::White, text("(   ).  "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   /")),
                              color(Color::Palette16::White, text("(___(__) "))));
        result.push_back(color(Color::Palette16::White, text("     *  *  * ")));
        result.push_back(color(Color::Palette16::White, text("    *  *  *  ")));
    } else if (WMO_code == 86) {
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" _`/\"\"")),
                              color(Color::Palette16::White, text(".-.    "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("  ,\\_")),
                              color(Color::Palette16::White, text("(   ).  "))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text("   /")),
                              color(Color::Palette16::White, text("(___(__) "))));
        result.push_back(color(Color::Palette16::White, text("    * * * *  ")));
        result.push_back(color(Color::Palette16::White, text("   * * * *   ")));
    } else if (WMO_code == 95) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(hbox(color(Color::Palette16::BlueLight, text("  ‚‘,")),
                              color(Color::Palette16::YellowLight, text("ϟ")),
                              color(Color::Palette16::BlueLight, text(",‘‚")),
                              color(Color::Palette16::YellowLight, text("ϟ"))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" ϟ")),
                              color(Color::Palette16::BlueLight, text("‚’‚")),
                              color(Color::Palette16::YellowLight, text("ϟ")),
                              color(Color::Palette16::BlueLight, text(",’, "))));
    } else if (WMO_code == 96 || WMO_code == 99) {
        result.push_back(color(Color::Palette16::White, text("     .-.     ")));
        result.push_back(color(Color::Palette16::White, text("    (   ).   ")));
        result.push_back(color(Color::Palette16::White, text("   (___(__)  ")));
        result.push_back(hbox(color(Color::Palette16::BlueLight, text("  ‚")),
                              color(Color::Palette16::White, text("◦")),
                              color(Color::Palette16::BlueLight, text(",")),
                              color(Color::Palette16::YellowLight, text("ϟ")),
                              color(Color::Palette16::BlueLight, text(",")),
                              color(Color::Palette16::White, text("◦")),
                              color(Color::Palette16::BlueLight, text("‚")),
                              color(Color::Palette16::YellowLight, text("ϟ"))));
        result.push_back(hbox(color(Color::Palette16::YellowLight, text(" ϟ")),
                              color(Color::Palette16::BlueLight, text(",")),
                              color(Color::Palette16::White, text("◦")),
                              color(Color::Palette16::BlueLight, text("‚")),
                              color(Color::Palette16::YellowLight, text("ϟ")),
                              color(Color::Palette16::BlueLight, text(",")),
                              color(Color::Palette16::White, text("◦")),
                              color(Color::Palette16::BlueLight, text("‚ "))));
    } else {
        result.push_back(color(Color::Palette16::White, text("    .-.      ")));
        result.push_back(color(Color::Palette16::White, text("     __)     ")));
        result.push_back(color(Color::Palette16::White, text("    (        ")));
        result.push_back(color(Color::Palette16::White, text("     `-’     ")));
        result.push_back(color(Color::Palette16::White, text("      •      ")));
    }
    return vbox(result);
}

ftxui::Element WeatherForecast::PartsOfDayForecast(uint8_t day_number) const noexcept {
    using namespace ftxui;
    auto result = Elements();
    for (uint8_t day_part = 1; day_part <= PARTS_OF_DAY_PER_DAY; day_part++) {
        auto cur_part = Elements();
        cur_part.push_back(center(text(DAY_PART_STRING[day_part - 1])));
        cur_part.push_back(separator());
        auto picture = ForecastPicture(
                static_cast<uint8_t>(current_city_->second.json["hourly"]["weathercode"][day_number * HOURS_PER_DAY +
                                                                                         day_part *
                                                                                         HOURS_PER_PART_OF_DAY + 3]));
        auto text_block = Elements();
        auto it = WEATHERCODE_STRING.find(
                static_cast<uint8_t>(current_city_->second.json["hourly"]["weathercode"][day_number * HOURS_PER_DAY +
                                                                                         day_part *
                                                                                         HOURS_PER_PART_OF_DAY + 3]));
        if (it != WEATHERCODE_STRING.end()) {
            text_block.push_back(text(it->second));
        } else {
            text_block.push_back(text("Не определено  "));
        }

        float temperature = 0;
        float apparent_temperature = 0;
        size_t wind_degree = 0;
        size_t min_wind_speed = SIZE_MAX;
        size_t max_wind_speed = 0;
        size_t visibility = 0;
        size_t relativehumidity = 0;
        size_t precipitation = 0;
        size_t precipitation_probability = 0;

        for (uint8_t i = 0; i < HOURS_PER_PART_OF_DAY; i++) {
            temperature += static_cast<float>(current_city_->second.json["hourly"]["temperature_2m"][
                    day_number * HOURS_PER_DAY +
                    day_part *
                    HOURS_PER_PART_OF_DAY + i]);
            apparent_temperature += static_cast<float>(current_city_->second.json["hourly"]["apparent_temperature"][
                    day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]);
            wind_degree += static_cast<size_t>(current_city_->second.json["hourly"]["winddirection_10m"][
                    day_number * HOURS_PER_DAY +
                    day_part *
                    HOURS_PER_PART_OF_DAY + i]);
            min_wind_speed = std::min(min_wind_speed,
                                      static_cast<size_t>(current_city_->second.json["hourly"]["windspeed_10m"][
                                              day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]));
            max_wind_speed = std::max(max_wind_speed,
                                      static_cast<size_t>(current_city_->second.json["hourly"]["windspeed_10m"][
                                              day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]));
            visibility += static_cast<size_t>(current_city_->second.json["hourly"]["visibility"][
                    day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]);
            relativehumidity += static_cast<size_t>(current_city_->second.json["hourly"]["relativehumidity_2m"][
                    day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]);
            precipitation += static_cast<size_t>(current_city_->second.json["hourly"]["precipitation"][
                    day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]);
            precipitation_probability += static_cast<size_t>(current_city_->second.json["hourly"]["precipitation_probability"][
                    day_number * HOURS_PER_DAY + day_part * HOURS_PER_PART_OF_DAY + i]);
        }
        temperature /= static_cast<float>(HOURS_PER_PART_OF_DAY);
        apparent_temperature /= static_cast<float>(HOURS_PER_PART_OF_DAY);
        int temperature_int = std::ceil(temperature);
        int apparent_temperature_int = std::ceil(apparent_temperature);
        wind_degree /= static_cast<size_t>(HOURS_PER_PART_OF_DAY);
        wind_degree /= 45;
        visibility /= static_cast<size_t>(HOURS_PER_PART_OF_DAY);
        visibility /= 1000;
        relativehumidity /= static_cast<size_t>(HOURS_PER_PART_OF_DAY);
        precipitation /= static_cast<size_t>(HOURS_PER_PART_OF_DAY);
        precipitation_probability /= static_cast<size_t>(HOURS_PER_PART_OF_DAY);

        text_block.push_back(text((temperature > 0 ? "+" : "") + std::to_string(temperature_int) + "(" +
                                  std::to_string(apparent_temperature_int) + ") " +
                                  static_cast<std::string>(current_city_->second.json["hourly_units"]["apparent_temperature"])));
        text_block.push_back(hbox(text(WIND_DIRECTION[wind_degree]), text(" "), color(Color::Palette16::GreenLight,
                                                                                      text(std::to_string(
                                                                                              min_wind_speed))),
                                  (max_wind_speed == min_wind_speed ? text("") : hbox(text("-"),
                                                                                      color(Color::Palette16::YellowLight,
                                                                                            text(std::to_string(
                                                                                                    max_wind_speed))))),
                                  text(" км/ч")));
        text_block.push_back(
                hbox(text(std::to_string(visibility)), text(" км | "), text(std::to_string(relativehumidity)),
                     text(" %")));
        text_block.push_back(hbox(text(std::to_string(precipitation)), text(" мм | "),
                                  text(std::to_string(precipitation_probability)), text(" %")));

        cur_part.push_back(hbox(picture, vbox(text_block)));
        result.push_back(vbox(cur_part) | flex);

        if (day_part != PARTS_OF_DAY_PER_DAY) {
            result.push_back(separator());
        }
    }
    return hbox(result);
}

ftxui::Element WeatherForecast::CurrentForecast() const noexcept {
    using namespace ftxui;
    auto picture = ForecastPicture(
            static_cast<uint8_t>(current_city_->second.json["current_weather"]["weathercode"]));
    auto text_block = Elements();
    auto it = WEATHERCODE_STRING.find(
            static_cast<uint8_t>(current_city_->second.json["current_weather"]["weathercode"]));
    if (it != WEATHERCODE_STRING.end()) {
        text_block.push_back(text(it->second));
    } else {
        text_block.push_back(text("Не определено  "));
    }

    int temperature = std::ceil(static_cast<float>(current_city_->second.json["current_weather"]["temperature"]));
    auto wind_degree = static_cast<size_t>(current_city_->second.json["current_weather"]["winddirection"]) / 45;
    auto wind_speed = static_cast<size_t>(current_city_->second.json["current_weather"]["windspeed"]);

    text_block.push_back(text((temperature > 0 ? "+" : "") + std::to_string(temperature) + " " +
                              static_cast<std::string>(current_city_->second.json["hourly_units"]["apparent_temperature"])));
    text_block.push_back(hbox(text(WIND_DIRECTION[wind_degree]), text(" "),
                              color(Color::Palette16::YellowLight, text(std::to_string(wind_speed))), text(" км/ч")));


    return vbox(text(""), hbox(picture, vbox(text_block)));
}

void WeatherForecast::GenerateForecast() noexcept {
    using namespace ftxui;
    forecast_.clear();
    if (current_city_->second.status == NotACity) {
        forecast_ = {text(""), bold(color(Color::Palette16::Red, text("Not a city.")))};
    } else if (current_city_->second.status == NothingParsed || current_city_->second.status == CityCoordinatesParsed) {
        forecast_ = {text(""), bold(color(Color::Palette16::Red, text("Bad connection.")))};
    } else {
        forecast_.reserve(days_count_ + 1);
        forecast_.push_back(CurrentForecast());
        for (uint8_t i = 0; i < days_count_; i++) {
            forecast_.push_back(window(center(text(ParseDate(i))), PartsOfDayForecast(i)));
        }
    }
}

bool WeatherForecast::IncreaseDays() noexcept {
    if (days_count_ == MAX_DAYS_COUNT) {
        return false;
    }
    days_count_++;
    if (current_city_->second.status == WeatherForecastParsed) {
        forecast_.push_back(
                window(center(ftxui::text(ParseDate(days_count_ - 1))), PartsOfDayForecast(days_count_ - 1)));
    }
    return true;
}

bool WeatherForecast::DecreaseDays() noexcept {
    if (days_count_ == MIN_DAYS_COUNT) {
        return false;
    }
    days_count_--;
    if (current_city_->second.status == WeatherForecastParsed) {
        forecast_.pop_back();
    }
    return true;
}

bool WeatherForecast::IncreaseCity() noexcept {
    if (current_city_ == end_city_) {
        return false;
    }
    current_city_++;
    forecast_.clear();
    GenerateForecast();
    return true;
}

bool WeatherForecast::DecreaseCity() noexcept {
    if (current_city_ == cities_.begin()) {
        return false;
    }
    current_city_--;
    forecast_.clear();
    GenerateForecast();
    return true;
}

WeatherForecast::WeatherForecast(const std::string& file_name) : WeatherForecast() {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw InvalidConfigPathException();
    }
    auto json_parsed = nlohmann::json::parse(file);
    if (json_parsed.contains("days_count")) {
        days_count_ = json_parsed["days_count"];
    }
    if (json_parsed.contains("update_rate")) {
        update_rate_ = json_parsed["update_rate"];
    }
    if (json_parsed.contains("cities")) {
        for (size_t i = 0; i < json_parsed["cities"].size(); i++) {
            std::string city_name = to_string(json_parsed["cities"][i]);
            std::reverse(city_name.begin(), city_name.end());
            city_name.pop_back();
            std::reverse(city_name.begin(), city_name.end());
            city_name.pop_back();
            cities_[city_name] = CityForecast();
        }
    }
    if (cities_.empty()) {
        throw NoCitiesException();
    }
    current_city_ = cities_.begin();
    end_city_ = cities_.begin();
    for (size_t i = 0; i < cities_.size() - 1; i++) {
        end_city_++;
    }
}

void WeatherForecast::Start() noexcept {
    using namespace ftxui;
    UpdateData();
    GenerateForecast();
    std::atomic<bool> running = true;
    auto screen = ScreenInteractive::TerminalOutput();
    auto renderer = Renderer([this]() -> Element {
        return vbox({hbox(text("Прогноз погоды: "), text(current_city_->first)), vbox(forecast_)});
    });
    auto component = CatchEvent(renderer, [&](const Event& event) -> bool {
        if (event == Event::Character('p') || event == Event::ArrowLeft) {
            return DecreaseCity();
        } else if (event == Event::Character('n') || event == Event::ArrowRight) {
            return IncreaseCity();
        } else if (event == Event::Escape) {
            running = false;
            screen.ExitLoopClosure()();
            return true;
        } else if (event == Event::Character('+') || event == Event::ArrowUp) {
            return IncreaseDays();
        } else if (event == Event::Character('-') || event == Event::ArrowDown) {
            return DecreaseDays();
        } else if (event == Event::F5 || event == Event::Custom) {
            UpdateData();
            GenerateForecast();
            return true;
        }
        return false;
    });
    std::thread timer([&] {
        while (running) {
            for (size_t i = 0; i < update_rate_; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (!running) {
                    break;
                }
            }
            if (!running) {
                break;
            }
            screen.PostEvent(Event::Custom);
        }
    });
    screen.Loop(component);
    timer.join();
}

void WeatherForecast::DeleteNotACities() noexcept {
    auto current_city_name = current_city_->first;
    std::vector<std::string> cities_to_delete;
    for (auto it = cities_.begin(); it != cities_.end(); it++) {
        if (it->second.status == NotACity) {
            cities_to_delete.push_back(it->first);
        }
    }
    for (size_t i = 0; i < cities_to_delete.size(); i++) {
        cities_.erase(cities_to_delete[i]);
    }
    current_city_ = cities_.find(current_city_name);
    end_city_ = cities_.begin();
    for (size_t i = 0; i < cities_.size() - 1; i++) {
        end_city_++;
    }
}
