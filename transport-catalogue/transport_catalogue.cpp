#include "transport_catalogue.h"

namespace transport_catalogue {


	const domain::Stop& TransportCatalogue::AddStop(domain::Stop&& stop) {
		stops_.push_back(stop);
		stop_name_to_stops_and_stop_buses_[stops_.back().name] = { &stops_.back(), {} };
		return stops_.back();
	}

	const domain::Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
		const auto result = stop_name_to_stops_and_stop_buses_.find(stop_name);
		return (result == stop_name_to_stops_and_stop_buses_.end()) ? nullptr : result->second.first;

	}

	const domain::Bus& TransportCatalogue::AddBus(domain::Bus&& bus) {
		buses_.push_back(bus);
		bus_name_to_buses_[buses_.back().name] = &buses_.back();

		for (const auto& bus_stop : buses_.back().bus_stops) {
			stop_name_to_stops_and_stop_buses_[bus_stop->name].second.insert(buses_.back().name);
		}

		return buses_.back();
	}

	const domain::Bus* TransportCatalogue::GetBus(const std::string_view bus_name) const {
		const auto result = bus_name_to_buses_.find(bus_name);
		return (result == bus_name_to_buses_.end()) ? nullptr : result->second;

	}

	void TransportCatalogue::AddDistanceBetweenStops(std::string_view first_stop,
		std::string_view second_stop, int distance) {
		stops_to_stop_to_distance_[detail::StopToStop{ GetStop(first_stop),
			GetStop(second_stop) }] = distance;
	}

	int TransportCatalogue::GetDistanceBetweenStops(std::string_view first_stop,
		std::string_view second_stop) const {
		auto iter = stops_to_stop_to_distance_.find(detail::StopToStop{ GetStop(first_stop),
			GetStop(second_stop) });
		if (iter != stops_to_stop_to_distance_.end()) {

			return iter->second;
		}
		iter = stops_to_stop_to_distance_.find(detail::StopToStop{ GetStop(second_stop),
			GetStop(first_stop) });
		if (iter != stops_to_stop_to_distance_.end()) {

			return iter->second;
		}
		return 0;
	}

	int TransportCatalogue::CalculateRouteLength(std::vector<const domain::Stop*> bus_stops) const {
		int route_length = 0;

		for (size_t i = 1; i < bus_stops.size(); ++i)
		{
			route_length += GetDistanceBetweenStops(bus_stops[i - 1]->name,
				bus_stops[i]->name);
		};

		return route_length;
	}
}