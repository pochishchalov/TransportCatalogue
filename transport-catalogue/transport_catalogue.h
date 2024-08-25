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

		// Структура для доступа к расстоянию между остановками в словаре TransportCatalogue
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

		// Добавляет "действительное" расстояние между остановками
		void AddDistanceBetweenStops(std::string_view first_stop,
			std::string_view second_stop, int distance);

		// Возвращает "действительное" расстояние между остановками
		int GetDistanceBetweenStops(std::string_view first_stop,
			std::string_view second_stop) const;

		// Возвращает длину маршрута
		int CalculateRouteLength(std::vector<const domain::Stop*> bus_stops) const;

		// Возвращает "набор" названий автобусных маршрутов проходящих через остановку 
		const std::set<std::string_view>& GetBusesByStop(std::string_view stop_name) const
		{ return stop_name_to_stops_and_stop_buses_.at(stop_name).second; };

		// Возвращает вектор уникальных указателей всех автобусных маршрутов отсортированный по названию
		const std::vector<const domain::Bus*> GetUniqueBuses() const;

		// Возвращает вектор уникальных указателей всех остановок отсортированный по названию
		const std::vector<const domain::Stop*> GetUniqueStops() const;

		// Возвращает информацию о маршруте (запрос Bus)
		const std::optional<detail::RouteInformation> GetRouteInformation(const std::string_view bus_name) const;

		// Возвращает маршруты, проходящие через остановку (запрос Stop)
		const std::optional<std::set<std::string_view>> GetStopInformation(const std::string_view stop_name) const;

		int GetBusWaitTime() const { return bus_wait_time_; } 
		double GetBusVelocity() const { return bus_velocity_; }

		void SetBusWaitTime(const int bus_wait_time) { bus_wait_time_ = bus_wait_time; }
		void SetBusVelocity(const double bus_velocity) { bus_velocity_ = bus_velocity; }

	private:
		
		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;

		int bus_wait_time_ = 0;
		double bus_velocity_ = 0.0;

		// Словарь хранящий указатели на остановки и "набор" названий автобусов проходящих через эту остановку с доступом по имени остановки
		std::unordered_map<std::string_view, std::pair<const domain::Stop*,
			std::set<std::string_view>>> stop_name_to_stops_and_stop_buses_;

		//  Словарь хранящий указатели на автобусные маршруты с доступом по его имени
		std::unordered_map<std::string_view, const domain::Bus*> bus_name_to_buses_;

		//  Словарь хранящий "действительное" расстояния между остановками с доступом по структуре StopToStop
		std::unordered_map<detail::StopToStop, int, detail::StopToStopHash> stops_to_stop_to_distance_;

	};
}