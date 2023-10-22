#pragma once 

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_router.h"
#include "router.h"
#include "graph.h"

#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>

#include <iostream>

void SerializeTransportCatalogue(std::ostream& out, const transport_catalogue::TransportCatalogue& db,
                                  const RenderSettings& render_settings, const RoutingSettings& routing_settings);
void DeserializeTransportCatalogue(std::istream& in, transport_catalogue::TransportCatalogue& transport_catalogue,
                                   RenderSettings& render_settings, RoutingSettings& routing_settings);

svg_serialize::Color TransformToSerializeColor(const svg::Color& color);
render_settings_serialize::RenderSettings SerializeRenderSettings(const RenderSettings& settings);
svg::Color TransformToSvgColor(const svg_serialize::Color& color_serialize);
void DeserializeRenderSettings(const render_settings_serialize::RenderSettings& settings_serialize,
                                            RenderSettings& settings);
transport_router_serialize::RoutingSettings SerializeRoutingSettings(const RoutingSettings& settings);
void DeserializeRoutingSettings(const transport_router_serialize::RoutingSettings& settings_serialize, RoutingSettings& settings);

