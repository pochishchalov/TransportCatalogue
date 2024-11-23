
#include <iostream>
#include <fstream>

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

    const auto doc_json = reader.GetInfo(handler);

    svg::Document doc_svg = handler.RenderMap();

    ofstream out_json("out.json"s);

    if (!out_json) {
        cerr << "невозможно открыть файл 'out.json' для вывода"s << endl;
    }
    else {
        json::Print(doc_json, out_json);
    }

    ofstream out_svg("out_image.svg"s);

    if (!out_svg) {
        cerr << "невозможно открыть файл 'out.json' для вывода"s << endl;
    }
    else {
        doc_svg.Render(out_svg);
    }

}