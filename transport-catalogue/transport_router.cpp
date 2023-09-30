#include "transport_router.h"
#include "transport_catalogue.h"
#include "router.h"

#include <vector>
#include <utility>
#include <optional>

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& db,
                                 const RoutingSettings& settings) 
    : db_(db)
    , transport_router_(InitializeInternalData(settings)) {
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

TransportRouter::Graph TransportRouter::InitializeInternalData(const RoutingSettings& settings) {
    Graph graph(db_.GetStopCount() * 2);
    auto buses = db_.GetBuses();
    
    for (const auto& bus: buses) {
        for (size_t i = 0; i != bus.route.size(); ++i) {
            int span_count = 0;
            
            RouteItem edge_description{
                bus.route.at(i)->name,
                bus.route.at(i)->name,
                bus.name,
                span_count,
                static_cast<double>(settings.bus_wait_time)
            };
            
            edge_descriptions_.push_back(edge_description);
            
            size_t stop1_id = db_.GetStopIdByName(edge_description.from);
            size_t stop1_dup_id = stop1_id + db_.GetStopCount();
            
            graph::Edge<double> edge{
                stop1_id,
                stop1_dup_id,
                edge_description.time
            };
            
            graph.AddEdge(edge);
            
            double time = 0;
            for (size_t j = i + 1; j != bus.route.size(); ++j) {
                ++span_count;
                auto distance = db_.GetDistanceBetweenStops(bus.route.at(j -1), bus.route.at(j));
                time += distance / (settings.bus_velocity * 1000. / 60);
                
                RouteItem edge_description{
                    bus.route.at(i)->name,
                    bus.route.at(j)->name,
                    bus.name,
                    span_count,
                    time
                };
                
                edge_descriptions_.push_back(edge_description);
                
                graph::Edge<double> edge{
                    stop1_dup_id,
                    db_.GetStopIdByName(edge_description.to),
                    edge_description.time
                };
                
                graph.AddEdge(edge);
            }
        }
    }
    return graph;
}

