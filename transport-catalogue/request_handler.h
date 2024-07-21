#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <set>

namespace handler {

    class RequestHandler {
    public:

        RequestHandler(const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::MapRenderer& renderer)
            :catalogue_(catalogue), renderer_(renderer)
        {
        }

        // Для хранения уникальных маршрутов в лексиграфическом порядке по названию
        using BusesContainer = std::set<const domain::Bus*, domain::BusPtrNameCompare>;

        // Возвращает набор (set) указателей на автобусный маршрут отсортированный по названию
        const BusesContainer GetBusesPtr() const;

        // Визуализирует карту маршрутов с использованием MapRenderer
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue& catalogue_;
        const renderer::MapRenderer& renderer_;
    };

} // namespace handler
