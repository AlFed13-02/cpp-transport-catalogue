#pragma once

#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <istream>

using namespace std::literals;

namespace transport_catalogue {
namespace detail {
template <typename IStream>
int ReadLineWithNumber(IStream& input_stream){
    int result;
    input_stream >> result;
    input_stream >> std::ws;
    return result;
}

std::string Strip(std::string str);

struct StringPairHasher {
    std::size_t operator()(const std::pair<std::string, std::string>& string_pair) const;
};
}

namespace input {
std::tuple<double, double, std::unordered_map<std::string, int>> ParseStopData(const std::string& raw_data);
std::vector<std::string> ParseBusData(const std::string& raw_data);

template <typename IStream>
void FillTransportCatalogue(TransportCatalogue& transport_catalogue, IStream& input_stream){
    int input_query_count = detail::ReadLineWithNumber(input_stream);
    
    std::unordered_map<std::string, std::string> bus_raw_data;
    std::unordered_map<std::string, std::string> stop_raw_data;
    
    for (int i = 0; i != input_query_count; ++i) {
        std::string query_type, name;
        input_stream >> query_type;
        std::getline(input_stream, name, ':');
        name = detail::Strip(name);
        
        std::string raw_data;
        std::getline(input_stream, raw_data);
        if (query_type == "Stop"s) {
            stop_raw_data[name] = std::move(raw_data);
        } else if (query_type == "Bus"s) {
            bus_raw_data[name] = std::move(raw_data);
        }
    }
    
    std::unordered_map<const std::pair<std::string, std::string>, int, detail::StringPairHasher> stop_to_stop_distance;
    for (const auto& [name, data]: stop_raw_data) {
        auto [lat, lng, distance_to_stops] = ParseStopData(data);
        transport_catalogue.AddStop(name, lat, lng);
        
        
        for (const auto& [stop_name, distance]: distance_to_stops) {
            stop_to_stop_distance[std::make_pair(name, stop_name)] = distance;
        }
    }
    
    for (const auto& [name, data]: bus_raw_data) {
        auto stop_names = ParseBusData(data);
        transport_catalogue.AddBus(name, stop_names);
    }
    
    for (const auto& [stop_to_stop_pair, distance]: stop_to_stop_distance) {
        auto stop1 = transport_catalogue.FindStop(stop_to_stop_pair.first);
        auto stop2 = transport_catalogue.FindStop(stop_to_stop_pair.second);
        transport_catalogue.SetDistanceBetweenStops(stop1, stop2, distance);
        }
}
}
}