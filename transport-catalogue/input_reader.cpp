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
            for (size_t i = 0; i < commands_.size(); ++i) {
                if (commands_[i].command == "Stop") {
                    catalogue.AddStop({ commands_[i].id, detail::ParseCoordinates(commands_[i].description) });
                }
                else if (commands_[i].command == "Bus") {
                    bus_commands_pos.push_back(i);
                }
            }

            for (const auto bus_command_pos : bus_commands_pos) {
                std::vector<const TransportCatalogue::Stop*> bus_stops;
                for (const auto stop_name : detail::ParseRoute(commands_[bus_command_pos].description)) {
                    bus_stops.push_back(catalogue.GetStop(stop_name));
                }
                catalogue.AddBus({ commands_[bus_command_pos].id, std::move(bus_stops) });
            }
        }
    }
}