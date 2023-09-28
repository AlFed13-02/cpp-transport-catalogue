#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <string_view>
#include <map>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain { 
struct Stop;
struct Bus;
    
namespace detail {
struct StopPairHasher {
    std::size_t operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const;
};
    
struct BusPtrCompare {
    bool operator()(const Bus* lhs, const Bus* rhs) const;
};
    
struct StopPtrCompare {
    bool operator()(const Stop* lhs, const Stop* rhs) const;
};
}
    
struct Stop {
    std::string name;
    geo::Coordinates coordinates; 
    
    Stop(std::string name, double lat, double lng);
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
    bool is_roundtrip;
};

struct BusStat {
    std::string name;
    int stop_count;
    int unique_stop_count;
    double route_length;
    double curvature;
};
    
struct StopStat {
    std::string name;
    std::set<std::string_view> buses;
};
    
struct MapStat {
    const std::set<const Bus*, detail::BusPtrCompare> buses;
    const std::set<const Stop*, detail::StopPtrCompare> stops;
};
    
struct BusBaseRequest {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip;
};
    
struct StopBaseRequest {
    std::string name;
    double lat;
    double lng;
    std::unordered_map<std::string, int> distances;
};
    
struct RouteItem {
    std::string from;
    std::string to;
    std::string bus;
    int span_count;
    double time;
};
    
struct RoutingSettings {
    int bus_wait_time = 0;
    int bus_velocity = 0;
};
}
