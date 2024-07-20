#include "request_handler.h"

#include <set>
#include <unordered_set>

namespace handler {

	using namespace std;

	// Возвращает количество уникальных остановок в векторе
	int UnigueStopsCount(std::vector<const domain::Stop*> bus_stops) {
		std::unordered_set<std::string_view> unique_stops;
		for (const auto stop : bus_stops) {
			unique_stops.insert(stop->name);
		}
		return static_cast<int>(unique_stops.size());
	}

	// Возвращает "кратчайшую" длину маршрута 
	double CalculateGeoDistance(std::vector<const domain::Stop*> bus_stops) {
		double geo_distance = 0.0;
		for (size_t i = 1; i < bus_stops.size(); ++i)
		{
			geo_distance += geo::ComputeDistance(bus_stops[i - 1]->coordinates,
				bus_stops[i]->coordinates);
		};
		return geo_distance;
	}

	const optional<RouteInformation> RequestHandler::GetRouteInformation(const string_view bus_name) const {
		if (catalogue_.GetBus(bus_name)) {
			const auto& found_bus_stops = catalogue_.GetBus(bus_name)->bus_stops;
			int route_length = catalogue_.CalculateRouteLength(found_bus_stops);

			return RouteInformation{ static_cast<int>(found_bus_stops.size()),
				UnigueStopsCount(found_bus_stops), route_length,
				 route_length / CalculateGeoDistance(found_bus_stops) };
		}
		else {
			return {};
		}
	}

	const optional<set<string_view>> RequestHandler::GetStopInformation(const string_view stop_name) const {

		if (catalogue_.GetStop(stop_name)) {
			return catalogue_.GetBusesByStop(stop_name);
		}
		else {
			return {};
		}
	}

	// Для хранения уникальных маршрутов в лексиграфическом порядке по названию
	using AllBusesPtr = std::set<const domain::Bus*, domain::BusPtrNameCompare>;

	const AllBusesPtr RequestHandler::GetBusesPtr() const {
		std::set<const domain::Bus*, domain::BusPtrNameCompare> result;
		const auto& buses = catalogue_.GetBuses();
		for (const auto& bus : buses) {
			result.insert(bus.second);
		}
		return result;
	}

	svg::Document RequestHandler::RenderMap() const {
		
		return renderer_.Render(GetBusesPtr());
	}

} // namespace handler