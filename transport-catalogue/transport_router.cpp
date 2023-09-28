#include "transport_router.h"
#include "transport_catalogue.h"
#include "router.h"

#include <vector>
#include <utility>
#include <optional>

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& db,
                                 const transport_catalogue::TransportCatalogue::Graph graph,
                                 const std::vector<domain::RouteItem> edge_descriptions) 
    : db_(db)
    , transport_router_(graph)
    , edge_descriptions_(edge_descriptions) {
}

std::optional<TransportRouter::RouteInfo> TransportRouter::BuildRoute(std::string from, std::string to) const {
    auto from_id = db_.GetStopIdByName(from);
    auto to_id = db_.GetStopIdByName(to);
    auto route = transport_router_.BuildRoute(from_id, to_id);
    
    if (!route) {
        return std::nullopt;
    }
    
    RouteInfo route_info;
    route_info.total_time = route->weight;
    for (auto edge_id: route->edges) {
        route_info.items.push_back(edge_descriptions_.at(edge_id));
    }
    return route_info;
}

