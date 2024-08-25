#include "transport_catalogue.h"

#include <algorithm>
#include <optional>
#include <set>
#include <unordered_set>

namespace transport_catalogue {

	using namespace std;

	// Возвращает количество уникальных остановок в векторе
	int UnigueStopsCount(vector<const domain::Stop*> bus_stops) {
		auto arr = bus_stops;

		std::sort(arr.begin(), arr.end(), [](const domain::Stop* lhs, const domain::Stop* rhs) {
			return lhs->name < rhs->name;
			});
		arr.erase(std::unique(arr.begin(), arr.end()), arr.end());

		return static_cast<int>(arr.size());
	}

	// Возвращает "кратчайшую" длину маршрута 
	double CalculateGeoDistance(vector<const domain::Stop*> bus_stops) {
		double geo_distance = 0.0;
		for (size_t i = 1; i < bus_stops.size(); ++i)
		{
			geo_distance += geo::ComputeDistance(bus_stops[i - 1]->coordinates,
				bus_stops[i]->coordinates);
		};
		return geo_distance;
	}

	const domain::Stop& TransportCatalogue::AddStop(domain::Stop&& stop) {
		stops_.push_back(stop);
		stop_name_to_stops_and_stop_buses_[stops_.back().name] = { &stops_.back(), {} };
		return stops_.back();
	}

	const domain::Stop* TransportCatalogue::GetStop(const string_view stop_name) const {
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

	const domain::Bus* TransportCatalogue::GetBus(const string_view bus_name) const {
		const auto result = bus_name_to_buses_.find(bus_name);
		return (result == bus_name_to_buses_.end()) ? nullptr : result->second;

	}

	void TransportCatalogue::AddDistanceBetweenStops(string_view first_stop,
		string_view second_stop, int distance) {
		stops_to_stop_to_distance_[detail::StopToStop{ GetStop(first_stop),
			GetStop(second_stop) }] = distance;
	}

	int TransportCatalogue::GetDistanceBetweenStops(string_view first_stop,
		string_view second_stop) const {
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

	int TransportCatalogue::CalculateRouteLength(vector<const domain::Stop*> bus_stops) const {
		int route_length = 0;

		for (size_t i = 1; i < bus_stops.size(); ++i)
		{
			route_length += GetDistanceBetweenStops(bus_stops[i - 1]->name,
				bus_stops[i]->name);
		};

		return route_length;
	}

	const optional<detail::RouteInformation> TransportCatalogue::GetRouteInformation(const string_view bus_name) const {
		if (GetBus(bus_name)) {
			const auto& found_bus_stops = GetBus(bus_name)->bus_stops;
			int route_length = CalculateRouteLength(found_bus_stops);

			return detail::RouteInformation{ static_cast<int>(found_bus_stops.size()),
				UnigueStopsCount(found_bus_stops), route_length,
				 route_length / CalculateGeoDistance(found_bus_stops) };
		}
		else {
			return {};
		}
	}

	const optional<set<string_view>> TransportCatalogue::GetStopInformation(const string_view stop_name) const {

		if (GetStop(stop_name)) {
			return GetBusesByStop(stop_name);
		}
		else {
			return {};
		}
	}

	const vector<const domain::Bus*> TransportCatalogue::GetUniqueBuses() const {
		const auto& buses = bus_name_to_buses_;

		vector<const domain::Bus*> result;
		result.reserve(buses.size());

		for (const auto& bus : buses) {
			result.push_back(bus.second);
		}
		std::sort(result.begin(), result.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
			return lhs->name < rhs->name;
			});
		return result;
	}

	const vector<const domain::Stop*> TransportCatalogue::GetUniqueStops() const {
		std::vector<const domain::Stop*> result;
		for (const auto& bus : GetUniqueBuses()) {
			result.insert(result.end(), bus->bus_stops.begin(), bus->bus_stops.end());
		}
		std::sort(result.begin(), result.end(), [](const domain::Stop* lhs, const domain::Stop* rhs) {
			return lhs->name < rhs->name;
			});
		result.erase(std::unique(result.begin(), result.end()), result.end());
		return result;
	}
}