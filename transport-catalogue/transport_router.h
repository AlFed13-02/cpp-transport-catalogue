#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"

#include <string>
#include <vector>
#include <optional>

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

class TransportRouter {
public:
    using Graph = graph::DirectedWeightedGraph<double>;
    
    TransportRouter(const transport_catalogue::TransportCatalogue& db,
                    const RoutingSettings& settings);
    
    struct RouteInfo {
        std::vector<RouteItem> items;
        double total_time;
    };
    
    std::optional<RouteInfo> BuildRoute(std::string from, std::string to) const;
    
private:
    const transport_catalogue::TransportCatalogue& db_;
    //Graph graph_;
    std::vector<RouteItem> edge_descriptions_;
    graph::Router<double> transport_router_;
    
    Graph InitializeInternalData(const RoutingSettings& settings);
    
};