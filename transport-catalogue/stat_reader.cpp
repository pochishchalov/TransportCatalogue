#include "stat_reader.h"

#include <iostream>
#include <iomanip>

namespace transport_catalogue {

    using namespace std::literals;

    namespace detail {

        void PrintBusStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {

            output << "Bus "s << request << ": "s;
            if (tansport_catalogue.GetBus(request)) {
                const auto info = tansport_catalogue.GetRouteInformation(request);
                output << info.stops_count << " stops on route, "s << info.unique_stops_count
                    << " unique stops, "s << std::setprecision(6) << info.route_length << " route length"s << '\n';
            }
            else {
                output << "not found"s << '\n';
            }
        }

        void PrintStopStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {

            output << "Stop "s << request << ": "s;
            if (tansport_catalogue.GetStop(request)) {
                const auto info = tansport_catalogue.GetStopInformation(request);
                if (info.empty()) {
                    output << "no buses"s << '\n';
                }
                else {
                    output << "buses"s;
                    for (const auto bus_name : info) {
                        output << ' ' << bus_name;
                    }
                    output << '\n';
                }
            }
            else {
                output << "not found"s << '\n';
            }
        }

    }

    namespace reader {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {

            auto space_pos = request.find(' ');
            std::string_view command = request;
            command.remove_suffix(command.size() - space_pos);
            request.remove_prefix(space_pos + 1);

            if (command == "Bus"s) {
                detail::PrintBusStat(tansport_catalogue, request, output);
            }

            else if (command == "Stop"s) {
                detail::PrintStopStat(tansport_catalogue, request, output);
            }
        }

        void GetStat(const TransportCatalogue& tansport_catalogue, std::istream& input,
            std::ostream& output) {

            std::string line;
            std::getline(input, line);
            int stat_request_count = std::stoi(line);

            for (int i = 0; i < stat_request_count; ++i) {
                std::getline(input, line);
                ParseAndPrintStat(tansport_catalogue, line, output);
            }
        }
    }
}