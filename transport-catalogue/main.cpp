#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

//using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        transport_catalogue::TransportCatalogue transport_catalogue;
        RequestHandler request_handler(transport_catalogue);
        JsonReader json_reader(std::cin, request_handler);
        json_reader.ProcessBaseRequests();
        auto render_settings = json_reader.GetRenderSettings();
        auto routing_settings = json_reader.GetRoutingSettings();
        
        std::ofstream out(json_reader.GetSerializationFileName(), std::ios::binary);
        SerializeTransportCatalogue(out, transport_catalogue, render_settings, routing_settings);
    } else if (mode == "process_requests"sv) {
        transport_catalogue::TransportCatalogue transport_catalogue;
        RequestHandler request_handler(transport_catalogue);
        JsonReader json_reader(std::cin, request_handler);
        RenderSettings render_settings;
        RoutingSettings routing_settings;
        
        std::ifstream in(json_reader.GetSerializationFileName(), std::ios::binary);
        DeserializeTransportCatalogue(in, transport_catalogue, render_settings, routing_settings);
        TransportRouter transport_router(transport_catalogue, routing_settings);
        auto json_doc = json_reader.ProcessStatRequests(render_settings, transport_router);
        json::Print(json_doc, std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}