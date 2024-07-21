
#include <iostream>
#include <string>

#include "json_reader.h"

using namespace std;

int main() {

    transport_catalogue::TransportCatalogue catalogue;

    reader::JsonReader reader(std::cin);

    reader.AddBaseRequests(catalogue);

    renderer::MapRenderer renderer(reader.GetRenderSettings());

    handler::RequestHandler handler(catalogue, renderer);

    const auto doc = reader.GetInfo(handler, catalogue);

    json::Print(doc, std::cout);

}