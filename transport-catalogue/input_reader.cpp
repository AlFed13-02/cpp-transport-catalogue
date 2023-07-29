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
}
}