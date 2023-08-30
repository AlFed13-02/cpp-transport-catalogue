#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

#include <iostream>

using namespace std::literals;

int main() {
    transport_catalogue::TransportCatalogue transport_catalogue;
    RequestHandler request_handler(transport_catalogue);
    JsonReader json_reader(request_handler);
    json_reader.LoadJson(std::cin);
    json_reader.ProcessBaseRequests();
    auto json_doc = json_reader.ProcessStatRequests();
    json::Print(json_doc, std::cout);
}
