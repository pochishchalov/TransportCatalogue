#include "transport_catalogue.h"

#include <unordered_set>

namespace transport_catalogue {

	namespace detail {

		int UnigueStopsCount(std::vector<const Stop*> bus_stops) {
			std::unordered_set<std::string_view> unique_stops;
			for (const auto stop : bus_stops) {
				unique_stops.insert(stop->name);
			}
			return static_cast<int>(unique_stops.size());
		}

		double CalculateGeoDistance(std::vector<const Stop*> bus_stops) {
			double geo_distance = 0.0;
			for (size_t i = 1; i < bus_stops.size(); ++i)
			{
				geo_distance += geo::ComputeDistance(bus_stops[i - 1]->coordinates,
					bus_stops[i]->coordinates);
			};
			return geo_distance;
		}
	}

	const Stop& TransportCatalogue::AddStop(Stop&& stop) {
		stops_.push_back(stop);
		stop_name_to_stops_and_stop_busses_[stops_.back().name] = { &stops_.back(), {} };
		return stops_.back();
	}

	const Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
		return (stop_name_to_stops_and_stop_busses_.count(stop_name)) ?
			stop_name_to_stops_and_stop_busses_.at(stop_name).first : nullptr;
	}

	const Bus& TransportCatalogue::AddBus(Bus&& bus) {
		buses_.push_back(bus);
		bus_name_to_busses_[buses_.back().name] = &buses_.back();

		for (const auto& bus_stop : buses_.back().bus_stops) {
			stop_name_to_stops_and_stop_busses_[bus_stop->name].second.insert(buses_.back().name);
		}

		return buses_.back();
	}

	const Bus* TransportCatalogue::GetBus(const std::string_view bus_name) const {
		return (bus_name_to_busses_.count(bus_name)) ?
			bus_name_to_busses_.at(bus_name) : nullptr;
	}

	void TransportCatalogue::AddDistanceBetweenStops(std::string_view first_stop,
		std::string_view second_stop, int distance) {
		stops_to_stop_to_distance_[detail::StopToStop{ GetStop(first_stop),
			GetStop(second_stop) }] = distance;
	}

	int TransportCatalogue::GetDistanceBetweenStops(std::string_view first_stop,
		std::string_view second_stop) const {
		if (stops_to_stop_to_distance_.count(detail::StopToStop{ GetStop(first_stop),
			GetStop(second_stop) })) {

			return stops_to_stop_to_distance_.at(detail::StopToStop{ GetStop(first_stop),
				GetStop(second_stop) });
		}
		if (stops_to_stop_to_distance_.count(detail::StopToStop{ GetStop(second_stop),
			GetStop(first_stop) })) {

			return stops_to_stop_to_distance_.at(detail::StopToStop{ GetStop(second_stop),
				GetStop(first_stop) });
		}
		return 0;
	}

	int TransportCatalogue::CalculateRouteLength(std::vector<const Stop*> bus_stops) const {
		int route_length = 0;

		for (size_t i = 1; i < bus_stops.size(); ++i)
		{
			route_length += GetDistanceBetweenStops(bus_stops[i - 1]->name,
				bus_stops[i]->name);
		};

		return route_length;
	}

	const detail::RouteInformation TransportCatalogue::GetRouteInformation(const std::string_view bus_name) const {
		const auto& found_bus_stops = GetBus(bus_name)->bus_stops;
		int route_length = CalculateRouteLength(found_bus_stops);

		return { static_cast<int>(found_bus_stops.size()), detail::UnigueStopsCount(found_bus_stops), route_length,
			 route_length / detail::CalculateGeoDistance(found_bus_stops) };
	}

	const std::set<std::string_view> TransportCatalogue::GetStopInformation(const std::string_view stop_name) const {

		return (GetStop(stop_name)) ? stop_name_to_stops_and_stop_busses_.at(stop_name).second
			: std::set<std::string_view>{};
	}
}