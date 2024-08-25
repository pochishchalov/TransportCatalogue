#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <vector>

namespace handler {

class RequestHandler {

private:

    using StopInformation = std::set < std::string_view>;
    using RouteInformation = transport_catalogue::detail::RouteInformation;
    using RouterInformation = router::RouterInformation;

public:
        
    RequestHandler(const transport_catalogue::TransportCatalogue& catalogue,
        const renderer::MapRenderer& renderer , const router::TransportRoute& router)
        :catalogue_(catalogue)
        ,renderer_(renderer)
        ,router_(router)
    {
    }

    // Возвращает вектор указателей на автобусный маршрут отсортированный по названию
    const renderer::BusesContainer GetBusesPtr() const;

    // Возвращает вектор указателей на остановки отсортированный по названию
    const renderer::StopsContainer GetStopsPtr() const;

    // Возвращает маршруты, проходящие через остановку (запрос Stop)
    const std::optional<StopInformation> GetStopInformation(const std::string_view stop_name) const;

    // Возвращает информацию о маршруте (запрос Bus)
    const std::optional<RouteInformation> GetRouteInformation(const std::string_view bus_name) const;

    // Визуализирует карту маршрутов с использованием MapRenderer
    svg::Document RenderMap() const;

    // Возвращает иформацию по маршруту из TransportRoute
    const std::optional<RouterInformation> GetRouterInfo(std::string_view from, std::string_view to) const;

private:

     // RequestHandler использует агрегацию объектов "Транспортный Справочник", "Визуализатор Карты" и "Транспортный маршрутизатор"

    const transport_catalogue::TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRoute& router_;
};

} // namespace handler
