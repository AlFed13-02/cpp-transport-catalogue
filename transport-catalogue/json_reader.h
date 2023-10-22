#pragma once

#include "json.h"
#include "request_handler.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json_builder.h"

#include <iostream>
#include <string>
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

class JsonReader {
public:
    JsonReader(std::istream& input, RequestHandler& request_handler);
    
    void ProcessBaseRequests() const;
    json::Document ProcessStatRequests(const RenderSettings& render_settings, const TransportRouter& transport_router) const;
    std::string GetSerializationFileName() const;
    RenderSettings GetRenderSettings() const;
    RoutingSettings GetRoutingSettings() const;
    
private:
    json::Document doc_;
    RequestHandler& request_handler_;
    
    domain::StopBaseRequest ExtractStopBaseRequest(const json::Dict& request) const;
    domain::BusBaseRequest ExtractBusBaseRequest(const json::Dict& request) const;
    
    static svg::Color TransformToColor(const json::Node& node);
    
    void BuildResponseForStopRequest(const json::Dict& request, json::Builder& builder) const;
    void BuildResponseForBusRequest(const json::Dict& request, json::Builder& builder) const;
    void BuildResponseForMapRequest(json::Builder& builder, const RenderSettings& settings) const;
    void BuildResponseForRouteRequest(const json::Dict& request, json::Builder& builder, const TransportRouter& router) const;
};