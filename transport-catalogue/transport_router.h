#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <vector>
#include <unordered_map>

namespace router {

using namespace std::literals;

struct GraphWeight
{
	double time = 0.0;
	int span_count = 0;
	std::string_view bus_name;
};

bool operator<(const GraphWeight& lhs, const GraphWeight& rhs);
bool operator>(const GraphWeight& lhs, const GraphWeight& rhs);
GraphWeight operator+(const GraphWeight& lhs, const GraphWeight& rhs);

enum RouteType { WAIT, BUS };

struct RouteItem
{
	RouteType type;
	int span_count = 0;
	double time = 0.0;
	std::string_view data;
};

struct RouterInformation {
	double total_time = 0.0;
	std::vector<RouteItem> items;
};

class TransportRoute {
private:

	using Catalogue = transport_catalogue::TransportCatalogue;
	using Graph = graph::DirectedWeightedGraph<GraphWeight>;
	using GraphEdge = graph::Edge<GraphWeight>;

public:

	TransportRoute(const Catalogue& catalogue)
		:bus_wait_time_(static_cast<double>(catalogue.GetBusWaitTime()))
		,time_coef_(60 / (catalogue.GetBusVelocity() * 1000))
		,graph_(BuildGraph(catalogue))
		,router_(graph_)
	{
	}

	const std::optional<RouterInformation> GetRouteInfo(std::string_view from, std::string_view to) const;

private:

	double bus_wait_time_ = 0.0;
	double time_coef_ = 0.0;

	// Контейнеры для хранения остановок (дублеры в контейнерах не хранятся)
	std::unordered_map<std::string_view, size_t> stops_to_index_;
	std::vector<std::string_view> index_to_stops_;

	// Вершины графа это остановки маршрутов TransportCatalogue и их дублеры
	// дублеры нужны для учета времени ожидания автобуса равное bus_wait_time_
	// индексы дублеров - четные, остановок - нечетные (индекс дублера + 1)
	Graph graph_;
	graph::Router<GraphWeight> router_;

	bool IsCorrectStop(std::string_view stop_name) const {
		return stops_to_index_.count(stop_name);
	}

	std::string_view GetStopName(size_t index) const {
		return index_to_stops_[index / 2];
	}

	size_t GetVertexIndex(std::string_view stop_name) const {
		return stops_to_index_.at(stop_name);
	}

	// Для доступа к индексу дублера
	size_t GetWaitVertexIndex(size_t index) const {
		return index - 1;
	}

	// Добавляет вершины и их дублеры, а так же ребра между ними в граф
	void AddVertexsToRoute(const Catalogue& catalogue, Graph& graph);

	// Добавляет вершину (остановку) в словарь stops_to_index_ и добавляет ребро между ними в граф
	void AddVertex(std::string_view stop, Graph& graph);

	// Добавляет ребра от остановки start_range до всех остановок диапазона start_range + 1 ... end_range
	template <typename It>
	void AddStopEdges(It start_range, It end_range, const Catalogue& catalogue, std::string_view bus_name, Graph& graph) {
		const auto start_stop = *start_range;
		int weight_sum = 0, span_counter = 0;

		for (auto lhs = start_range, rhs = start_range + 1; rhs != end_range; ++lhs, ++rhs) {
			++span_counter;
			const auto lhs_stop = *lhs, rhs_stop = *rhs;

			weight_sum += catalogue.GetDistanceBetweenStops(lhs_stop->name, rhs_stop->name);
			double edge_weight = 0.0;
			(start_range == rhs) ? edge_weight = bus_wait_time_
				: edge_weight = weight_sum * time_coef_;

			graph.AddEdge(GraphEdge{
				.from = GetVertexIndex(start_stop->name),
				.to = GetWaitVertexIndex(GetVertexIndex(rhs_stop->name)),
				.weight = {.time = edge_weight, .span_count = span_counter, .bus_name = bus_name} });
		}
	}

	// Добавляет ребра конкретного маршрута Bus в граф
	void AddBusEdges(const domain::Bus* bus, const Catalogue& catalogue, Graph& graph);

	// Добавляет в граф ребра между вершинами
	void AddEdges(const Catalogue& catalogue, Graph& graph);

	// Строит граф по маршрутами из TransportCatalogue
	Graph BuildGraph(const Catalogue& catalogue);

	// Создает RouteItem из ребра графа
	RouteItem CreateRouteItem(size_t edge_index) const;
};

} // namespace router