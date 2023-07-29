#pragma once

#include "transport_catalogue.h"
#include "input_reader.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <iterator>
#include <istream>

using namespace std::literals;


namespace transport_catalogue {
namespace output {
template <typename OStream>
void PrintRouteInfo(const std::string& name, const TransportCatalogue& transport_catalogue, OStream& output_stream){
    output_stream << "Bus "s << name << ": "s;
    auto route_info = transport_catalogue.GetBusInfo(name);
    if (route_info) {
        output_stream << route_info.value().stop_count << " stops on route, "s 
            << route_info.value().unique_stop_count << " unique stops, "s
            << std::setprecision(6) << route_info.value().distance << " route length, "s 
            << route_info.value().curvature << " curvature"s;
    } else {
        output_stream << "not found"s;
    }
    output_stream << std::endl;
}
    
template <typename OStream>
void PrintBusesPassingThroughStop(const std::string& name, const TransportCatalogue& transport_catalogue, OStream& output_stream) {
    output_stream << "Stop "s << name << ": "s;
    const auto buses = transport_catalogue.GetBusesPassingThroughStop(name);
    if (buses) {
        if (buses.value().empty()) {
            output_stream << "no buses"s;
        } else {
            output_stream << "buses "s << *buses.value().begin();
            for (auto it = next(buses.value().begin()); it != buses.value().end(); ++it) {
                output_stream << " "s << *it;
            }
        }
    } else {
        output_stream << "not found"s;
    }
    output_stream << std::endl; 
}

template <typename IStream, typename OStream>
void ProcessRequests(const TransportCatalogue& transport_catalogue, IStream& input_stream, OStream& output_stream) {
    int input_query_count = detail::ReadLineWithNumber(input_stream);
    
    for (int i = 0; i < input_query_count; ++i) {
        std::string request_type, name;
        input_stream >> request_type;
        std::getline(input_stream, name);
        name = detail::Strip(name);
        
        if (request_type == "Bus"s) {
            PrintRouteInfo(name, transport_catalogue, output_stream);
        } else if (request_type == "Stop"s) {
            PrintBusesPassingThroughStop(name, transport_catalogue, output_stream);
        } 
    }
}
}
}