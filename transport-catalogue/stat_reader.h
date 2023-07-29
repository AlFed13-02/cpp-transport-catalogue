#pragma once

#include "transport_catalogue.h"

#include <set>
#include <tuple>

namespace transport_catalogue {
namespace output {
void PrintRouteInfo(const std::tuple<int, int, double, double>& route_info);
void PrintStopsPassingThroughStop(const std::set<std::string_view> buses);

void ProcessRequests(const TransportCatalogue& transport_catalogue);
}
}