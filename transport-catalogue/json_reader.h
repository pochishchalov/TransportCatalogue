#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"

namespace reader {

    class JsonReader {
    public:

        JsonReader(std::istream& input);

        // Наполняет TransportCatalogue информацией из "base_requests"
        void AddBaseRequests(transport_catalogue::TransportCatalogue& catalogue);

        // Возвращает RoutingSettings из словаря "routing_settings"
        void AddRoutingSettings(transport_catalogue::TransportCatalogue& catalogue);

        // Возвращает Document по запросу "stat_requests"
        json::Document GetInfo(const handler::RequestHandler& handler);

        // Возвращает MapRendererSettings из словаря "render_settings"
        renderer::MapRendererSettings GetRenderSettings() const;

    private:

        json::Document document_;
    };

} // namespace reader