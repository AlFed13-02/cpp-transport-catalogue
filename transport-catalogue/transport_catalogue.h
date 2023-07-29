#pragma once

#include "geo.h"

#include <unordered_map>
#include <string>
#include <deque>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

namespace transport_catalogue {
struct Stop {
    std::string name;
    geo::Coordinates coordinates; 
    
    Stop(std::string name, double lat, double lng);
};
    
namespace detail {
struct StopPairHasher {
    std::size_t operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const;
};
}

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    
    const Stop* AddStop(const std::string& name, double lat, double lng);
    const Stop* FindStop(std::string_view name) const;
    const std::optional<std::set<std::string_view>> GetBusesPassingThroughStop(std::string_view name) const;
    void SetDistanceBetweenStops(const Stop* stop1, const Stop* stop2, int distance);
    double GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const;
    
   void AddBus(const std::string& name, const std::vector<std::string>& stop_names);
   const Bus* FindBus(std::string_view name) const;
   const std::optional<std::tuple<int, int, double, double>> GetBusInfo(std::string_view name) const;
   
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stops_lookup_;
    std::unordered_map<std::string_view, const Bus*> buses_lookup_;
    std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_; 
    std::unordered_map<const std::pair<const Stop*, const Stop*>, int, detail::StopPairHasher> real_distance_between_stops_;
};
}