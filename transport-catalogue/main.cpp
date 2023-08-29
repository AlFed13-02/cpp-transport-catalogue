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
}/*
#include "json_builder.h"
#include <iostream>


using namespace std;

int main() {
    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        }, cout
    );
    cout << endl;
    
    json::Print(
        json::Document{
            json::Builder{}
            .Value("just a string"s)
            .Build()
        },
        cout
    );
    cout << endl;
}*/