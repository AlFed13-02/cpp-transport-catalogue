#include "transport_catalogue.h"
#include "stat_reader.h"
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
void PrintRouteInfo(const std::tuple<int, int, double, double>& route_info) {
    auto [stop_count, unique_stop_count, distance, curvature] = route_info;
    std::cout << stop_count << " stops on route, "s 
        << unique_stop_count << " unique stops, "s
        << std::setprecision(6) << distance << " route length, "s 
        << curvature << " curvature"s << std::endl;
}

void PrintBusesPassingThroughStop(const std::set<std::string_view> buses) {
    if (buses.empty()) {
        std::cout << "no buses"s;
    } else {
        std::cout << "buses "s << *buses.begin();
        for (auto it = next(buses.begin()); it != buses.end(); ++it) {
            std::cout << " "s << *it;
        }
    }
}

void ProcessRequests(const TransportCatalogue& transport_catalogue) {
    int input_query_count = detail::ReadLineWithNumber();
    
    for (int i = 0; i < input_query_count; ++i) {
        std::string request_type, name;
        std::cin >> request_type;
        std::getline(std::cin, name);
        name = detail::Strip(name);
        
        if (request_type == "Bus"s) {
            const auto route_info = transport_catalogue.GetBusInfo(name);
            std::cout << "Bus "s << name << ": "s;
            if (route_info) {
                PrintRouteInfo(route_info.value());
            } else {
                std::cout << "not found"s << std::endl;
            }
        } else if (request_type == "Stop"s) {
            std::cout << "Stop "s << name << ": "s;
            const auto buses = transport_catalogue.GetBusesPassingThroughStop(name);
            if (buses) {
                PrintBusesPassingThroughStop(std::move(buses.value()));
            } else {
                    std::cout << "not found"s;
            }
            std::cout << std::endl;
        } 
    }
}
}
}