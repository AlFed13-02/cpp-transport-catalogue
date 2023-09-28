#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <string>
#include <unordered_map>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
namespace detail {
std::size_t StringPairHasher::operator()(const std::pair<std::string, std::string>& string_pair) const {
    return std::hash<std::string>{}(string_pair.first) + 37 * std::hash<std::string>{}(string_pair.second);
}
}

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& db)
    :db_(db) {}

const std::optional<domain::BusStat> RequestHandler::GetBusStat(std::string_view bus_name) const {
    return db_.GetBusStat(bus_name);
}
    
const std::optional<domain::StopStat> RequestHandler::GetStopStat(std::string_view stop_name) const {
    return db_.GetStopStat(stop_name);
}

void RequestHandler::HandleStopBaseRequests(const std::vector<domain::StopBaseRequest>& requests) {
    using RoadDistanceMap =  std::unordered_map<const std::pair<std::string, std::string>, int, detail::StringPairHasher>;
    RoadDistanceMap road_distances;
    
    for (const auto& request: requests) {
        db_.AddStop(request.name, request.lat, request.lng);
        
        for (const auto& [stop_name, distance]: request.distances) {
            road_distances[std::make_pair(request.name, stop_name)] = distance;
        }
    }
    
    for (const auto& [stop_pair, distance]: road_distances) {
        auto stop1 = db_.FindStop(stop_pair.first);
        auto stop2 = db_.FindStop(stop_pair.second);
        db_.SetDistanceBetweenStops(stop1, stop2, distance);
        } 
}

void RequestHandler::HandleBusBaseRequests(std::vector<domain::BusBaseRequest>& requests) {
    for (auto& request: requests) {
        if (!request.is_roundtrip) {
            request.stops.reserve(request.stops.size() * 2);
            for (auto it = request.stops.rbegin() + 1; it != request.stops.rend(); ++it) {
                request.stops.push_back(*it);
            }
        }
        db_.AddBus(request.name, request.stops, request.is_roundtrip);
    }
}

svg::Document RequestHandler::RenderRoutes(const RenderSettings& settings) const {
    auto coordinates = db_.GetStopsCoordinates();
    MapRenderer renderer(coordinates.begin(), coordinates.end(), settings);
    auto map_stat = db_.GetRoutesMapStat();
    return renderer.Render(map_stat);
}

TransportRouter RequestHandler::GetRouter(const domain::RoutingSettings& settings) const {
    const auto [graph, edge_descriptions] = db_.AsGraph(settings);
    return TransportRouter(db_, graph, edge_descriptions);
}