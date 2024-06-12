#include "transport_catalogue.h"

#include <unordered_set>

namespace transport_catalogue {

	namespace detail {

		int UnigueStopsCount(std::vector<const TransportCatalogue::Stop*> bus_stops) {
			std::unordered_set<std::string_view> unique_stops;
			for (const auto stop : bus_stops) {
				unique_stops.insert(stop->name);
			}
			return static_cast<int>(unique_stops.size());
		}

		double CalculateRouteLength(std::vector<const TransportCatalogue::Stop*> bus_stops) {
			double route_length = 0.0;
			for (size_t i = 1; i < bus_stops.size(); ++i)
			{
				route_length += geo::ComputeDistance(bus_stops[i - 1]->coordinates, bus_stops[i]->coordinates);
			};
			return route_length;
		}
	}

	const TransportCatalogue::Stop& TransportCatalogue::AddStop(TransportCatalogue::Stop&& stop) {
		stops_.push_back(stop);
		stop_name_to_stops_and_stop_busses[stops_.back().name] = { &stops_.back(), {} };
		return stops_.back();
	}

	const TransportCatalogue::Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
		return (stop_name_to_stops_and_stop_busses.count(stop_name)) ? stop_name_to_stops_and_stop_busses.at(stop_name).first : nullptr;
	}

	const TransportCatalogue::Bus& TransportCatalogue::AddBus(TransportCatalogue::Bus&& bus) {
		buses_.push_back(bus);
		bus_name_to_busses[buses_.back().name] = &buses_.back();

		for (const auto& bus_stop : buses_.back().bus_stops) {
			stop_name_to_stops_and_stop_busses[bus_stop->name].second.insert(buses_.back().name);
		}

		return buses_.back();
	}

	const TransportCatalogue::Bus* TransportCatalogue::GetBus(const std::string_view bus_name) const {
		return (bus_name_to_busses.count(bus_name)) ? bus_name_to_busses.at(bus_name) : nullptr;
	}

	const detail::RouteInformation TransportCatalogue::GetRouteInformation(const std::string_view bus_name) const {
		const auto& found_bus_stops = GetBus(bus_name)->bus_stops;
		return { static_cast<int>(found_bus_stops.size()), detail::UnigueStopsCount(found_bus_stops),
			detail::CalculateRouteLength(found_bus_stops) };
	}

	const std::set<std::string_view> TransportCatalogue::GetStopInformation(const std::string_view stop_name) const {
		return (GetStop(stop_name)) ? stop_name_to_stops_and_stop_busses.at(stop_name).second : std::set<std::string_view>{};
	}
}