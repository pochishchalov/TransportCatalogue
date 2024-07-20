#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <set>

namespace handler {

    struct RouteInformation {
        int stops_count = 0;
        int unique_stops_count = 0;
        int route_length = 0;
        double curvature = 0.0;
    };

    class RequestHandler {
    public:
        
        RequestHandler(const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::MapRenderer& renderer)
            :catalogue_(catalogue), renderer_(renderer)
        {
        }

        // Возвращает информацию о маршруте (запрос Bus)
        const std::optional<RouteInformation> GetRouteInformation(const std::string_view bus_name) const;

        // Возвращает маршруты, проходящие через остановку (запрос Stop)
        const std::optional<std::set<std::string_view>> GetStopInformation(const std::string_view stop_name) const;

        // Для хранения уникальных маршрутов в лексиграфическом порядке по названию
        using AllBusesPtr = std::set<const domain::Bus*, domain::BusPtrNameCompare>;

        // Возвращает набор (set) указателей на автобусный маршрут отсортированный по названию
        const AllBusesPtr GetBusesPtr() const;

        // Визуализирует карту маршрутов с использованием MapRenderer
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue& catalogue_;
        const renderer::MapRenderer& renderer_;
    };

} // namespace handler
