#include "json_reader.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <iostream>
#include <utility>
#include <vector>
#include <sstream>
#include <string>

using namespace std::literals;
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader(std::istream& input, RequestHandler& request_handler) 
    : doc_(std::move(json::Load(input)))
    , request_handler_(request_handler) {
}

void JsonReader::ProcessBaseRequests() const {
    auto requests = doc_.GetRoot().AsDict().at("base_requests"s);
    
    std::vector<domain::BusBaseRequest> bus_requests;
    std::vector<domain::StopBaseRequest> stop_requests;
    
    for (const auto& request: requests.AsArray()) {
        auto request_type = request.AsDict().at("type").AsString();
        if (request_type == "Stop"s){
            stop_requests.push_back(ExtractStopBaseRequest(request.AsDict()));
        } else {
            bus_requests.push_back(ExtractBusBaseRequest(request.AsDict()));
        }
    }
    
    request_handler_.HandleStopBaseRequests(stop_requests);
    request_handler_.HandleBusBaseRequests(bus_requests);
}

domain::StopBaseRequest JsonReader::ExtractStopBaseRequest(const json::Dict& request) const {
    auto name = request.at("name"s).AsString();
    auto lat = request.at("latitude"s).AsDouble();
    auto lng = request.at("longitude"s).AsDouble();
    
    std::unordered_map<std::string, int> distances;
    for (const auto& [key, value]: request.at("road_distances"s).AsDict()) {
       distances[key] = value.AsInt(); 
    }
    
    return domain::StopBaseRequest{name, lat, lng, distances};
}
    
domain::BusBaseRequest JsonReader::ExtractBusBaseRequest(const json::Dict& request) const {
    auto name = request.at("name"s).AsString();
    auto is_roundtrip = request.at("is_roundtrip"s).AsBool();
    
    std::vector<std::string> stops;
    for (const auto& stop_name: request.at("stops"s).AsArray()) {
        stops.push_back(stop_name.AsString());
    }
    
    return {name, stops, is_roundtrip};
}


json::Document JsonReader::ProcessStatRequests() const {
    auto routing_settings = GetRoutingSettings();
    auto router = request_handler_.GetRouter(routing_settings);
    
    auto requests = doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    
    json::Builder response_builder;
    response_builder.StartArray();
    for (const auto& request: requests) {
        json::Builder response_part_builder; 
        response_part_builder.StartDict()
            .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
        
        auto request_type = request.AsDict().at("type").AsString();
        if (request_type == "Bus"s) {
            BuildResponseForBusRequest(request.AsDict(), response_part_builder);
        } else if (request_type == "Stop"s) {
            BuildResponseForStopRequest(request.AsDict(), response_part_builder);
        } else if (request_type == "Map"s) {
            BuildResponseForMapRequest(response_part_builder);
        } else if (request_type == "Route"s) {
            BuildResponseForRouteRequest(request.AsDict(), response_part_builder, router);
        }
        
        response_builder.Value(response_part_builder.EndDict().Build().AsDict()).EndDict();     
    }
    return json::Document(response_builder.EndArray().Build());
}

RenderSettings JsonReader::GetRenderSettings() const {
    RenderSettings settings;
    auto json_settings = doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
    
    settings.width = json_settings.at("width"s).AsDouble();
    settings.height = json_settings.at("height"s).AsDouble();
    settings.padding = json_settings.at("padding"s).AsDouble();
    settings.line_width = json_settings.at("line_width"s).AsDouble();
    settings.stop_radius = json_settings.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = json_settings.at("bus_label_font_size"s).AsInt();
    settings.stop_label_font_size = json_settings.at("stop_label_font_size"s).AsInt();
    settings.underlayer_width = json_settings.at("underlayer_width"s).AsDouble();
    
    
    for (const auto& node: json_settings.at("bus_label_offset"s).AsArray()) {
        settings.bus_label_offset.push_back(node.AsDouble());
    }
    
    for (const auto& node: json_settings.at("stop_label_offset"s).AsArray()) {
        settings.stop_label_offset.push_back(node.AsDouble());
    }
    
    settings.underlayer_color = TransformToColor(json_settings.at("underlayer_color"s));
    
    for (const auto color: json_settings.at("color_palette"s).AsArray()) {
        settings.color_palette.push_back(TransformToColor(color));
    }
    
    return settings;
}

RoutingSettings JsonReader::GetRoutingSettings() const {
    RoutingSettings settings;
    auto json_settings = doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    
    settings.bus_wait_time = json_settings.at("bus_wait_time"s).AsInt();
    settings.bus_velocity = json_settings.at("bus_velocity"s).AsInt();
    
    return settings;
}

svg::Color JsonReader::TransformToColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }
    
    auto arr = node.AsArray();
    if (arr.size() == 3) {
        return svg::Rgb{static_cast<std::uint8_t>(arr[0].AsInt()), static_cast<std::uint8_t>(arr[1].AsInt()), static_cast<std::uint8_t>(arr[2].AsInt())};
        } 
            
    return svg::Rgba{static_cast<std::uint8_t>(arr[0].AsInt()), static_cast<std::uint8_t>(arr[1].AsInt()), static_cast<std::uint8_t>(arr[2].AsInt()), arr[3].AsDouble()};   
    
}

void JsonReader::BuildResponseForBusRequest(const json::Dict& request, json::Builder& builder) const {
    auto bus_stat = request_handler_.GetBusStat(request.at("name"s).AsString());
    if (bus_stat) {
        builder
            .Key("curvature"s).Value(bus_stat->curvature)
            .Key("route_length"s).Value(bus_stat->route_length)
            .Key("stop_count"s).Value(bus_stat->stop_count)
            .Key("unique_stop_count"s).Value(bus_stat->unique_stop_count);
    } else {
        builder.Key("error_message"s).Value("not found"s);
    }
}

void JsonReader::BuildResponseForStopRequest(const json::Dict& request, json::Builder& builder) const {
auto stop_stat = request_handler_.GetStopStat(request.at("name"s).AsString());
    if (stop_stat) {
        builder.Key("buses"s).StartArray();
        for (auto bus_name: stop_stat->buses) {
            builder.Value(std::string{bus_name});
        }
        builder.EndArray();
    } else {
        builder.Key("error_message"s).Value("not found"s);
    }         
}

void JsonReader::BuildResponseForMapRequest(json::Builder& builder) const {
    auto settings = GetRenderSettings();
    auto doc = request_handler_.RenderRoutes(settings);
    std::ostringstream out;
    doc.Render(out);
    builder.Key("map"s).Value(out.str()); 
}

void JsonReader::BuildResponseForRouteRequest(const json::Dict& request, json::Builder& builder, const TransportRouter& router) const {
    auto from = request.at("from"s).AsString();
    auto to = request.at("to"s).AsString();
    const auto route_info = router.BuildRoute(from, to);
    
    if (!route_info) {
        builder.Key("error_message"s).Value("not found"s);
    } else {
        builder
            .Key("total_time"s)
            .Value(route_info->total_time)
            .Key("items"s)
            .StartArray();
        for (const auto& item: route_info->items) {
            builder
                .StartDict()
                .Key("type"s);
                    
            if (item.span_count == 0) {
                builder
                    .Value("Wait"s)
                    .Key("stop_name"s) 
                    .Value(item.from);
            } else {
                builder
                    .Value("bus"s)
                    .Key("bus"s)
                    .Value(item.bus)
                    .Key("span_count"s)
                    .Value(item.span_count);
            }
                
            builder.Key("time"s).Value(item.time).EndDict();
        }  
        builder.EndArray();
    }
}