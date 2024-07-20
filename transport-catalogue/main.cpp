

#include <iostream>
#include <string>

#include "json_reader.h"

using namespace std;

int main() {
    
    transport_catalogue::TransportCatalogue catalogue;
   
    reader::JsonReader reader(std::cin);
            
    renderer::MapRenderer renderer(reader.GetReaderSettings());

    handler::RequestHandler handler(catalogue, renderer);

    reader.AddBaseRequests(catalogue);

    const auto doc = reader.GetInfo(handler);

    json::Print(doc, std::cout);
    
}