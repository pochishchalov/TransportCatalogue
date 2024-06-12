#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

	namespace detail {

		struct RouteInformation {
			int stops_count = 0;
			int unique_stops_count = 0;
			double route_length = 0.0;
		};
	}

	class TransportCatalogue {
	public:
		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
		};

		const Stop& AddStop(Stop&& stop);
		const Stop* GetStop(const std::string_view stop_name) const;

		struct Bus {
			std::string name;
			std::vector<const Stop*> bus_stops;
		};

		const Bus& AddBus(Bus&& bus);
		const Bus* GetBus(std::string_view bus_name) const;

		const detail::RouteInformation GetRouteInformation(const std::string_view bus_name) const;
		const std::set<std::string_view> GetStopInformation(const std::string_view stop_name) const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, std::pair<const Stop*, std::set<std::string_view>>> stop_name_to_stops_and_stop_busses;
		std::unordered_map<std::string_view, const Bus*> bus_name_to_busses;
	};
}