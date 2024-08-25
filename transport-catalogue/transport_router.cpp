#include "transport_router.h"

namespace router {

bool operator<(const GraphWeight& lhs, const GraphWeight& rhs) {
	return lhs.time < rhs.time;
}

bool operator>(const GraphWeight& lhs, const GraphWeight& rhs) {
	return rhs < lhs;
}

GraphWeight operator+(const GraphWeight& lhs, const GraphWeight& rhs) {
	GraphWeight temp = lhs;
	temp.time += rhs.time;
	temp.span_count += rhs.span_count;
	return temp;
}

const std::optional<RouterInformation> TransportRoute::GetRouteInfo(std::string_view from, std::string_view to) const {
	RouterInformation result;

	if (!IsCorrectStop(from) || !IsCorrectStop(to)) {
		return std::nullopt;
	}

	// Маршрут всегда начинается в дублере остановки from и заканивается в дублере остановки to
	const auto info = router_.BuildRoute(
		GetWaitVertexIndex(GetVertexIndex(from)), GetWaitVertexIndex(GetVertexIndex(to))
	);

	if (info.has_value()) {
		const auto info_value = info.value();
		result.total_time = info_value.weight.time;
		for (const auto edge_index : info_value.edges) {
			result.items.push_back(CreateRouteItem(edge_index));
		}
		return result;
	}
	else {
		return std::nullopt;
	}
}

void TransportRoute::AddVertexsToRoute(const Catalogue& catalogue, Graph& graph) {
	const auto stops = catalogue.GetUniqueStops();

	graph = Graph(stops.size() * 2);          // задаем размер графа
	index_to_stops_.resize(stops.size());     // и размер вектора
	for (const auto stop : stops) {
		AddVertex(stop->name, graph);
	}
}

void TransportRoute::AddVertex(std::string_view stop, Graph& graph) {

	size_t index = (stops_to_index_.size() * 2) + 1;

	stops_to_index_.insert({ stop, index });
	index_to_stops_[GetVertexIndex(stop) / 2] = stop;

	graph.AddEdge(GraphEdge{
		.from = GetWaitVertexIndex(GetVertexIndex(stop)),
		.to = GetVertexIndex(stop),
		.weight = {.time = bus_wait_time_, .bus_name = {}} });
}

void TransportRoute::AddBusEdges(const domain::Bus* bus, const Catalogue& catalogue, Graph& graph) {
	const auto stops_by_bus = bus->bus_stops;
	if (bus->is_roundtrip) {
		for (size_t i = 0; i < stops_by_bus.size() - 1; ++i) {
			AddStopEdges(stops_by_bus.begin() + i, stops_by_bus.end(), catalogue, bus->name, graph);
		}
	}
	else {
		size_t half_range = stops_by_bus.size() / 2;

		for (size_t i = 0; i < half_range; ++i) {
			AddStopEdges(stops_by_bus.begin() + i, stops_by_bus.end() - half_range, catalogue, bus->name, graph);
		}

		for (size_t i = half_range; i < stops_by_bus.size() - 1; ++i) {
			AddStopEdges(stops_by_bus.begin() + i, stops_by_bus.end(), catalogue, bus->name, graph);
		}
	}
}

TransportRoute::Graph TransportRoute::BuildGraph(const Catalogue& catalogue) {
	Graph result;
	AddVertexsToRoute(catalogue, result);
	AddEdges(catalogue, result);
	return result;
}

void TransportRoute::AddEdges(const Catalogue& catalogue, Graph& graph) {
	const auto buses = catalogue.GetUniqueBuses();
	for (const auto bus : buses) {
		AddBusEdges(bus, catalogue, graph);
	}
}

RouteItem TransportRoute::CreateRouteItem(size_t edge_index) const {
	const auto& edge = graph_.GetEdge(edge_index);
	if (edge.from / 2 == edge.to / 2) {
		return RouteItem{
			.type = RouteType::WAIT,
			.time = edge.weight.time,
			.data = GetStopName(edge.to),
		};
	}
	else {
		return RouteItem{
			.type = RouteType::BUS,
			.span_count = edge.weight.span_count,
			.time = edge.weight.time,
			.data = edge.weight.bus_name
		};
	}
}

} // namespace router