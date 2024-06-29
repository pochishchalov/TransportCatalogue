#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <numeric>
#include <unordered_set>

namespace transport_catalogue {

    namespace detail {

        /**
         * Удаляет пробелы в начале и конце строки
         */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Парсит остановку и возвращает пару строк (название и координаты, расстояние до других остановок с названиями)
         */
        std::pair<std::string_view, std::string_view> ParseStop(std::string_view str) {
            std::string_view coordinates = str;
            
            auto second_comma = str.find(',', str.find(',') + 1);

            if (second_comma == str.npos) {
                return { coordinates, {} };
            }

            coordinates.remove_suffix(coordinates.size() - second_comma);
            str.remove_prefix(second_comma + 1);

            return { Trim(coordinates), Trim(str)};
        }

        /**
         * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
         */
        geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return { lat, lng };
        }
        
        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
         * Парсит строку с расстоянием и названием остановки и возвращает пару (расстояние, название остановки)
         */
        std::vector<std::pair<int, std::string_view>> ParseDistanceToStop(std::string_view str) {
            std::vector<std::pair<int, std::string_view>> result;

            for (auto& distance_to_stop : Split(str, ',')) {
                std::string_view str_distance = distance_to_stop;
                str_distance.remove_suffix(str_distance.size() - str_distance.find('m'));
                int distance = std::stoi(std::string(str_distance));
                
                auto second_space = distance_to_stop.find(' ', distance_to_stop.find(' ') + 1);
                distance_to_stop.remove_prefix(second_space + 1);
                
                result.push_back({ distance, distance_to_stop });
            }

            return result;
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)) };
        }

    }

    namespace reader {

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = detail::ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
            std::vector<std::size_t> bus_commands_pos;
            bus_commands_pos.reserve(commands_.size());
            std::vector<std::tuple<std::string_view, std::string_view, int>> stop_to_stop_distance;
            stop_to_stop_distance.reserve(commands_.size());
            for (size_t i = 0; i < commands_.size(); ++i) {
                if (commands_[i].command == "Stop") {
                    auto stopcommand = detail::ParseStop(commands_[i].description);
                    catalogue.AddStop({ commands_[i].id, detail::ParseCoordinates(stopcommand.first) });
                    
                    if (!stopcommand.second.empty()) {
                        for (const auto& distance_to_stop : detail::ParseDistanceToStop(stopcommand.second)) {
                            stop_to_stop_distance.push_back({ commands_[i].id, distance_to_stop.second, distance_to_stop.first });
                        }
                    }
                    
                }
                else if (commands_[i].command == "Bus") {
                    bus_commands_pos.push_back(i);
                }
            }

            for (const auto& [first_stop, second_stop, distance] : stop_to_stop_distance) {
                catalogue.AddDistanceBetweenStops(first_stop, second_stop, distance);
            }

            for (const auto bus_command_pos : bus_commands_pos) {
                std::vector<const Stop*> bus_stops;
                for (const auto& stop_name : detail::ParseRoute(commands_[bus_command_pos].description)) {
                    bus_stops.push_back(catalogue.GetStop(stop_name));
                }
                catalogue.AddBus({ commands_[bus_command_pos].id, std::move(bus_stops) });
            }
            
        }

        void InputReader::LoadCommands(std::istream& input, TransportCatalogue& catalogue) {
            std::string line;
            std::getline(input, line);
            int base_request_count = std::stoi(line);
            for (int i = 0; i < base_request_count; ++i) {
                std::getline(input, line);
                ParseLine(line);
            }
            ApplyCommands(catalogue);
        }
    }
}