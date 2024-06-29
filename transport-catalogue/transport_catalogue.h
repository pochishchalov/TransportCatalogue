#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> bus_stops;
	};

	namespace detail {

		struct RouteInformation {
			int stops_count = 0;
			int unique_stops_count = 0;
			int route_length = 0;
			double curvature = 0.0;
		};

		struct StopToStop {
			const Stop* first_stop;
			const Stop* second_stop;
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

		const Stop& AddStop(Stop&& stop);
		const Stop* GetStop(const std::string_view stop_name) const;

		const Bus& AddBus(Bus&& bus);
		const Bus* GetBus(std::string_view bus_name) const;

		void AddDistanceBetweenStops(std::string_view first_stop,
			std::string_view second_stop, int distance);

		int GetDistanceBetweenStops(std::string_view first_stop,
			std::string_view second_stop) const;

		const detail::RouteInformation GetRouteInformation(const std::string_view bus_name) const;
		const std::set<std::string_view> GetStopInformation(const std::string_view stop_name) const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, std::pair<const Stop*,
			std::set<std::string_view>>> stop_name_to_stops_and_stop_busses_;

		std::unordered_map<std::string_view, const Bus*> bus_name_to_busses_;

		std::unordered_map<detail::StopToStop, int, detail::StopToStopHash> stops_to_stop_to_distance_;

		int CalculateRouteLength(std::vector<const Stop*> bus_stops) const;
	};
}