#pragma once

#include "geo.h"
#include "domain.h"

#include <unordered_map>
#include <string>
#include <deque>
#include <string_view>
#include <vector>
#include <optional>
#include <set>
#include <utility>

namespace transport_catalogue {
    
class TransportCatalogue {
public:
    using RoadDistances = std::unordered_map<const std::pair<const domain::Stop*, const domain::Stop*>, int, domain::detail::StopPairHasher>;
    
    TransportCatalogue() = default;
    
    void AddStop(const std::string& name, double lat, double lng);
    const domain::Stop* FindStop(std::string_view name) const;
    const std::optional<domain::StopStat> GetStopStat(std::string_view name) const;
    void SetDistanceBetweenStops(const domain::Stop* stop1, const domain::Stop* stop2, int distance);
    double GetDistanceBetweenStops(const domain::Stop* stop1, const domain::Stop* stop2) const;
    std::vector<geo::Coordinates> GetStopsCoordinates() const;
    size_t GetStopIdByName(const std::string& name) const;
    size_t GetStopCount() const;
    const std::deque<domain::Stop>& GetStops() const;
    
    void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip);
    const domain::Bus* FindBus(std::string_view name) const;
    const std::optional<domain::BusStat> GetBusStat(std::string_view name) const;
    domain::MapStat GetRoutesMapStat() const;
    const std::deque<domain::Bus>& GetBuses() const;
    
    const RoadDistances& GetRoadDistances() const;
   
private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;
    std::unordered_map<std::string_view, const domain::Stop*> stops_lookup_;
    std::unordered_map<std::string_view, const domain::Bus*> buses_lookup_;
    std::unordered_map<const domain::Stop*, std::set<std::string_view>> stop_to_buses_; 
    RoadDistances road_distances_;
};
}