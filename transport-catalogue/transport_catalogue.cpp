#include "transport_catalogue.h"
#include "geo.h"

#include <string>
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
Stop::Stop(std::string name, double lat, double lng)
    : name(name)
    , coordinates{lat, lng} {
}

namespace detail {
std::size_t StopPairHasher::operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const {
    return std::hash<const void*>{}(stop_pair.first) + 37 * std::hash<const void*>{}(stop_pair.second);
}
}
    
const Stop* TransportCatalogue::AddStop(const std::string& name, double lat, double lng) {
    stops_.emplace_back(name, lat, lng);
    stops_lookup_[stops_.back().name] = &stops_.back();
    stop_to_buses_.emplace(&stops_.back(), std::set<std::string_view>{});
    return &stops_.back();
} 

const Stop* TransportCatalogue::FindStop(std::string_view name) const{
    if (stops_lookup_.count(name)) {
        return stops_lookup_.at(name);
    }
    return nullptr;
}

const std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesPassingThroughStop(std::string_view name) const {
    auto stop = FindStop(name);
    
    if (!stop) {
        return std::nullopt;
    }
    
    return stop_to_buses_.at(stop);
}

double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop1, const Stop* stop2) const {
    if (real_distance_between_stops_.count(std::make_pair(stop1, stop2))) {
        return real_distance_between_stops_.at(std::make_pair(stop1, stop2));
    } else if (real_distance_between_stops_.count(std::make_pair(stop2, stop1))) {
        return real_distance_between_stops_.at(std::make_pair(stop2, stop1));
    } else {
        return geo::ComputeDistance(stop2->coordinates, stop1->coordinates);
    }
}

void TransportCatalogue::SetDistanceBetweenStops(const Stop* stop1, const Stop* stop2, int distance) {
    real_distance_between_stops_.emplace(std::make_pair(std::make_pair(stop1, stop2), distance));
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stop_names) {
    Bus bus;
    
    bus.name = name;
    
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

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    if (buses_lookup_.count(name)) {
        return buses_lookup_.at(name);
    }
    return nullptr;
}

const std::optional<std::tuple<int, int, double, double>> TransportCatalogue::GetBusInfo(std::string_view name) const {
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
    return std::tie(stop_count, unique_stop_count, real_distance, curvature);
}
}