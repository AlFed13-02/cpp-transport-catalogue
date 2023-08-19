#include "transport_catalogue.h"
#include "geo.h"
#include "domain.h"

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <numeric>
#include <functional>
#include <optional>
#include <unordered_set>
#include <set>
#include <string_view>

using namespace std::literals;

namespace transport_catalogue {
    
void TransportCatalogue::AddStop(const std::string& name, double lat, double lng) {
    stops_.emplace_back(name, lat, lng);
    stops_lookup_[stops_.back().name] = &stops_.back();
    stop_to_buses_.emplace(&stops_.back(), std::set<std::string_view>{});
} 

const domain::Stop* TransportCatalogue::FindStop(std::string_view name) const{
    if (stops_lookup_.count(name)) {
        return stops_lookup_.at(name);
    }
    return nullptr;
}

const std::optional<domain::StopStat> TransportCatalogue::GetStopStat(std::string_view name) const {
    auto stop = FindStop(name);
    
    if (!stop) {
        return std::nullopt;
    }
    
    return domain::StopStat{stop->name, stop_to_buses_.at(stop)};
}

double TransportCatalogue::GetDistanceBetweenStops(const domain::Stop* stop1, const domain::Stop* stop2) const {
    if (road_distances_.count(std::make_pair(stop1, stop2))) {
        return road_distances_.at(std::make_pair(stop1, stop2));
    } else if (road_distances_.count(std::make_pair(stop2, stop1))) {
        return road_distances_.at(std::make_pair(stop2, stop1));
    } else {
        return geo::ComputeDistance(stop2->coordinates, stop1->coordinates);
    }
}

void TransportCatalogue::SetDistanceBetweenStops(const domain::Stop* stop1, const domain::Stop* stop2, int distance) {
    road_distances_.emplace(std::make_pair(std::make_pair(stop1, stop2), distance));
}

std::vector<geo::Coordinates> TransportCatalogue::GetStopsCoordinates() const {
    std::vector<geo::Coordinates> result;
    for (const auto& [stop, buses]: stop_to_buses_) {
        if (!buses.empty()) {
            result.push_back(stop->coordinates);
        }
    }
    return result;
}
    
void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip) {
    domain::Bus bus;
    
    bus.name = name;
    bus.is_roundtrip = is_roundtrip;
    
    for (const std::string& name: stop_names) {
        auto stop = FindStop(name);
        bus.route.push_back(stop);
    }
    
    buses_.push_back(std::move(bus));
    buses_lookup_[buses_.back().name] = &buses_.back();
    
    for (const auto& stop: buses_.back().route) {
        stop_to_buses_[stop].insert(buses_.back().name);
    } 
}

const domain::Bus* TransportCatalogue::FindBus(std::string_view name) const {
    if (buses_lookup_.count(name)) {
        return buses_lookup_.at(name);
    }
    return nullptr;
}

const std::optional<domain::BusStat> TransportCatalogue::GetBusStat(std::string_view name) const {
    auto bus = FindBus(name);
    if (!bus) {
        return std::nullopt;
    }
    
    int stop_count = bus->route.size();
    std::unordered_set unique_stops(bus->route.begin(), bus->route.end());
    int unique_stop_count = unique_stops.size();
    
    auto geo_distance = std::transform_reduce(
        bus->route.begin() + 1,
        bus->route.end(),
        bus->route.begin(),
        0.,
        std::plus<>(),
        [](auto next, auto prev) {
            return geo::ComputeDistance(prev->coordinates, next->coordinates);
        });
    
    auto real_distance = std::transform_reduce(
        bus->route.begin() + 1,
        bus->route.end(),
        bus->route.begin(),
        0.,
        std::plus<>(),
        [this](auto next, auto prev) {
            return GetDistanceBetweenStops(prev, next);
        });
    
    auto curvature = real_distance / geo_distance;
    return domain::BusStat{bus->name, stop_count, unique_stop_count, real_distance, curvature};
}
    
domain::MapStat TransportCatalogue::GetRoutesMapStat() const {
    std::set<const domain::Bus*, domain::detail::BusPtrCompare> buses;
    for (const auto& bus: buses_) {
        if (!bus.route.empty()) {
            buses.insert(&bus);
        }
    }
    
    std::set<const domain::Stop*, domain::detail::StopPtrCompare> stops;
    for (const auto& [stop, buses]: stop_to_buses_) {
        if (!buses.empty()) {
            stops.insert(stop);
        }
    }
    return {buses, stops};
}
}