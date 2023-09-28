#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"

#include <string>
#include <vector>
#include <optional>

class TransportRouter {
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& db,
                    const transport_catalogue::TransportCatalogue::Graph graph,
                    const std::vector<domain::RouteItem> edge_descriptions);
    
    struct RouteInfo {
        std::vector<domain::RouteItem> items;
        double total_time;
    };
    
    std::optional<RouteInfo> BuildRoute(std::string from, std::string to) const;
    
private:
    const transport_catalogue::TransportCatalogue& db_;
    graph::Router<double> transport_router_;
    std::vector<domain::RouteItem> edge_descriptions_;
    
};