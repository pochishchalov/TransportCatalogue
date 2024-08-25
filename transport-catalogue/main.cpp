
#include <iostream>

#include "json_reader.h"

using namespace std;

int main() {
    
    transport_catalogue::TransportCatalogue catalogue;

    reader::JsonReader reader(cin);

    reader.AddBaseRequests(catalogue);
    reader.AddRoutingSettings(catalogue);

    renderer::MapRenderer renderer(reader.GetRenderSettings());
    router::TransportRoute route(catalogue);

    handler::RequestHandler handler(catalogue, renderer, route);

    const auto doc = reader.GetInfo(handler);

    json::Print(doc, cout);
}