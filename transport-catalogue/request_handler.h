#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string_view>
#include <unordered_set>
#include <vector>

using namespace std::literals;

namespace detail {
struct StringPairHasher {
    std::size_t operator()(const std::pair<std::string, std::string>& string_pair) const;
};
}

class RequestHandler {
public:
    RequestHandler(transport_catalogue::TransportCatalogue& db);
    
    const std::optional<domain::BusStat> GetBusStat(std::string_view bus_name) const;
    
    const std::optional<domain::StopStat> GetStopStat(std::string_view stop_name) const;
    
    void HandleBusBaseRequests(std::vector<domain::BusBaseRequest>& requests);
    void HandleStopBaseRequests(const std::vector<domain::StopBaseRequest>& requests);
    svg::Document RenderRoutes(const RenderSettings& settings) const;
    TransportRouter GetRouter(const RoutingSettings& settings) const;
    
private:
    transport_catalogue::TransportCatalogue& db_;
};