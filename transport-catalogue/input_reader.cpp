#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <utility>
#include <istream>
#include <unordered_map>

using namespace std::literals;

namespace transport_catalogue {
namespace detail {
int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    std::cin >> std::ws;
    return result;
}

std::string Strip(std::string str) {
    std::size_t pos_begin = str.find_first_not_of(' ');
    std::size_t pos_end = str.find_last_not_of(' ');
    return str.substr(pos_begin, pos_end - pos_begin + 1);
}

std::size_t StringPairHasher::operator()(const std::pair<std::string, std::string>& string_pair) const {
    return std::hash<std::string>{}(string_pair.first) + 37 * std::hash<std::string>{}(string_pair.second);
}
}

namespace input {
std::tuple<double, double, std::unordered_map<std::string, int>> ParseStopData(const std::string& raw_data) {
    double lat, lng;
    
    std::size_t begin_pos = 0;
    std::size_t comma_pos = raw_data.find_first_of(","s);
    lat = std::stod(detail::Strip(raw_data.substr(begin_pos, comma_pos - begin_pos)));
    
    begin_pos = comma_pos + 1;
    comma_pos = raw_data.find_first_of(","s, begin_pos);
    comma_pos = comma_pos == std::string::npos ? raw_data.size() : comma_pos ;
    lng = std::stod(detail::Strip(raw_data.substr(begin_pos, comma_pos - begin_pos)));
    
    std::unordered_map<std::string, int> distance_to_stops;
    begin_pos = comma_pos + 1;
    while (comma_pos != raw_data.size()) {
        std::string delim = "m to "s;
        size_t delim_pos = raw_data.find(delim, begin_pos);
        
        int distance = std::stoi(detail::Strip(raw_data.substr(begin_pos, delim_pos - begin_pos)));
        
        begin_pos = delim_pos + delim.size() - 1;
        comma_pos = raw_data.find_first_of(","s, begin_pos);
        comma_pos = comma_pos == std::string::npos ? raw_data.size() : comma_pos ;
        
        std::string stop_name = detail::Strip(raw_data.substr(begin_pos, comma_pos - begin_pos));
        distance_to_stops[stop_name] = distance; 
        begin_pos = comma_pos + 1;
    }
    return std::tie(lat, lng, distance_to_stops);
}

std::vector<std::string> ParseBusData(const std::string& raw_data) {
    std::size_t delim_pos = raw_data.find_first_of("->"s);
    char delim = raw_data[delim_pos];
    bool is_circular = delim == '>';
    
    std::size_t pos_begin = 0;
    std::size_t pos_end = delim_pos;
    std::vector<std::string> stop_names;
    while (pos_end != std::string::npos) {
        std::string stop_name = raw_data.substr(pos_begin, pos_end - pos_begin);
        stop_names.push_back(detail::Strip(stop_name));
        
        pos_begin = pos_end + 1;
        pos_end = raw_data.find_first_of(delim, pos_begin);
    }
    
    stop_names.push_back(detail::Strip(raw_data.substr(pos_begin, raw_data.size() - pos_begin)));
    
    if (!is_circular) {
        stop_names.reserve(stop_names.size() * 2);
        for (auto it = stop_names.rbegin() + 1; it != stop_names.rend(); ++it) {
            stop_names.push_back(*it);
        }
    }
    
    return stop_names;
}

void FillTransportCatalogue(TransportCatalogue& transport_catalogue) {
    int input_query_count = detail::ReadLineWithNumber();
    
    std::unordered_map<std::string, std::string> bus_raw_data;
    std::unordered_map<std::string, std::string> stop_raw_data;
    
    for (int i = 0; i != input_query_count; ++i) {
        std::string query_type, name;
        std::cin >> query_type;
        std::getline(std::cin, name, ':');
        name = detail::Strip(name);
        
        std::string raw_data;
        std::getline(std::cin, raw_data);
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