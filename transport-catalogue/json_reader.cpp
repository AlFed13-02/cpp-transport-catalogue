#include "json_reader.h"
#include "json.h"

#include <iostream>
#include <utility>
#include <vector>
#include <sstream>

using namespace std::literals;
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader(RequestHandler& request_handler) 
    : request_handler_(request_handler) {}

void JsonReader::LoadJson(std::istream& input) {
    doc_ = std::move(json::Load(input));
}

void JsonReader::ProcessBaseRequests() const {
    auto requests = doc_.GetRoot().AsMap().at("base_requests"s);
    
    std::vector<domain::BusBaseRequest> bus_requests;
    std::vector<domain::StopBaseRequest> stop_requests;
    
    for (const auto& request: requests.AsArray()) {
        auto request_type = request.AsMap().at("type").AsString();
        if (request_type == "Stop"s){
            stop_requests.push_back(ExtractStopBaseRequest(request.AsMap()));
        } else {
            bus_requests.push_back(ExtractBusBaseRequest(request.AsMap()));
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
    for (const auto& [key, value]: request.at("road_distances"s).AsMap()) {
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
    auto requests = doc_.GetRoot().AsMap().at("stat_requests"s).AsArray();
    
    json::Array responses;
    for (const auto& request: requests) {
        json::Dict response;
        response["request_id"s] = json::Node(request.AsMap().at("id"s).AsInt());
        
        auto request_type = request.AsMap().at("type").AsString();
        if (request_type == "Bus"s) {
            auto bus_stat = request_handler_.GetBusStat(request.AsMap().at("name"s).AsString());
            if (bus_stat) {
                response["curvature"s] = json::Node(bus_stat->curvature);
                response["route_length"s] = json::Node(bus_stat->route_length);
                response["stop_count"s] = json::Node(bus_stat->stop_count);
                response["unique_stop_count"s] = json::Node(bus_stat->unique_stop_count);
            } else {
                response["error_message"s] = json::Node("not found"s);
            }
        } else if (request_type == "Stop"s) {
            auto stop_stat = request_handler_.GetStopStat(request.AsMap().at("name"s).AsString());
            if (stop_stat) {
                json::Array buses;
                for (auto bus_name: stop_stat->buses) {
                    buses.push_back(json::Node(std::string{bus_name}));
                }
        
                response["buses"s] = json::Node(buses);
            } else {
                response["error_message"s] = json::Node("not found"s);
            }
        } else if (request_type == "Map"s) {
            auto settings = GetRenderSettings();
            auto doc = request_handler_.RenderRoutes(settings);
            std::ostringstream out;
            doc.Render(out);
            response["map"s] = json::Node(out.str()); 
        }
        responses.push_back(std::move(response));                                        
    }
    
    return json::Document(json::Node(responses));
}

RenderSettings JsonReader::GetRenderSettings() const {
    RenderSettings settings;
    auto json_settings = doc_.GetRoot().AsMap().at("render_settings"s).AsMap();
    
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