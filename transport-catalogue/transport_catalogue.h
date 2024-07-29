#pragma once

#include <deque>
#include <set>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace transport_catalogue {

	namespace detail {

		struct RouteInformation {
			int stops_count = 0;
			int unique_stops_count = 0;
			int route_length = 0;
			double curvature = 0.0;
		};

		// —труктура дл€ доступа к рассто€нию между остановками в словаре TransportCatalogue
		struct StopToStop {
			const domain::Stop* first_stop;
			const domain::Stop* second_stop;
			bool operator==(const StopToStop&) const = default;
		};

		struct StopToStopHash {
			std::size_t operator()(const StopToStop& stops) const {
				std::size_t hash_1 = reinterpret_cast<size_t>(stops.first_stop);
				std::size_t hash_2 = reinterpret_cast<size_t>(stops.second_stop);
				return hash_1 ^ (hash_2 << 1);
			}
		};

	}

	class TransportCatalogue {
	public:

		const domain::Stop& AddStop(domain::Stop&& stop);
		const domain::Stop* GetStop(const std::string_view stop_name) const;

		const domain::Bus& AddBus(domain::Bus&& bus);
		const domain::Bus* GetBus(std::string_view bus_name) const;

		// ƒобавл€ет "действительное" рассто€ние между остановками
		void AddDistanceBetweenStops(std::string_view first_stop,
			std::string_view second_stop, int distance);

		// ¬озвращает "действительное" рассто€ние между остановками
		int GetDistanceBetweenStops(std::string_view first_stop,
			std::string_view second_stop) const;

		// ¬озвращает длину маршрута
		int CalculateRouteLength(std::vector<const domain::Stop*> bus_stops) const;

		// ¬озвращает "набор" названий автобусных маршрутов проход€щих через остановку 
		const std::set<std::string_view>& GetBusesByStop(std::string_view stop_name) const
		{ return stop_name_to_stops_and_stop_buses_.at(stop_name).second; };

		// ¬озвращает словарь с автобусными маршрутами
		const std::unordered_map<std::string_view, const domain::Bus*> GetBuses() const 
		{ return bus_name_to_buses_; }

		// ¬озвращает информацию о маршруте (запрос Bus)
		const std::optional<detail::RouteInformation> GetRouteInformation(const std::string_view bus_name) const;

		// ¬озвращает маршруты, проход€щие через остановку (запрос Stop)
		const std::optional<std::set<std::string_view>> GetStopInformation(const std::string_view stop_name) const;

	private:
		
		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;

		// —ловарь хран€щий указатели на остановки и "набор" названий автобусов проход€щих через эту остановку с доступом по имени остановки
		std::unordered_map<std::string_view, std::pair<const domain::Stop*,
			std::set<std::string_view>>> stop_name_to_stops_and_stop_buses_;

		//  —ловарь хран€щий указатели на автобусные маршруты с доступом по его имени
		std::unordered_map<std::string_view, const domain::Bus*> bus_name_to_buses_;

		//  —ловарь хран€щий "действительное" рассто€ни€ между остановками с доступом по структуре StopToStop
		std::unordered_map<detail::StopToStop, int, detail::StopToStopHash> stops_to_stop_to_distance_;

	};
}