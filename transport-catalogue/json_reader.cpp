#include "json_reader.h"
#include "json.h"
#include "json_builder.h"

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
    auto requests = doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    
    json::Builder response_builder;
    response_builder.StartArray();
    for (const auto& request: requests) {
        json::Builder response_part_builder; 
        response_part_builder.StartDict()
            .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
        
        auto request_type = request.AsDict().at("type").AsString();
        if (request_type == "Bus"s) {
            auto bus_stat = request_handler_.GetBusStat(request.AsDict().at("name"s).AsString());
            if (bus_stat) {
                response_part_builder
                    .Key("curvature"s).Value(bus_stat->curvature)
                    .Key("route_length"s).Value(bus_stat->route_length)
                    .Key("stop_count"s).Value(bus_stat->stop_count)
                    .Key("unique_stop_count"s).Value(bus_stat->unique_stop_count);
            } else {
                response_part_builder.Key("error_message"s).Value("not found"s);
            }
        } else if (request_type == "Stop"s) {
            auto stop_stat = request_handler_.GetStopStat(request.AsDict().at("name"s).AsString());
            if (stop_stat) {
                response_part_builder.Key("buses"s).StartArray();
                for (auto bus_name: stop_stat->buses) {
                    response_part_builder.Value(std::string{bus_name});
                }
                response_part_builder.EndArray();
            } else {
                response_part_builder.Key("error_message"s).Value("not found"s);
            }
        } else if (request_type == "Map"s) {
            auto settings = GetRenderSettings();
            auto doc = request_handler_.RenderRoutes(settings);
            std::ostringstream out;
            doc.Render(out);
            response_part_builder.Key("map"s).Value(out.str()); 
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