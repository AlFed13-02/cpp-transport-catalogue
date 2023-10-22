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
#include <vector>
#include <string>
#include <variant>


svg_serialize::Color TransformToSerializeColor(const svg::Color& color) {
    svg_serialize::Color color_serialize;
    
    if (std::holds_alternative<std::monostate>(color)) {
		color_serialize.set_monostate(true);
	}
	else if (std::holds_alternative<std::string>(color)) {
		
		color_serialize.mutable_string_color()->set_color(std::get<std::string>(color));
	}
	else if (std::holds_alternative<svg::Rgb>(color)) {
		
		svg::Rgb c = std::get<svg::Rgb>(color);
		color_serialize.mutable_rgb()->set_red(c.red);
		color_serialize.mutable_rgb()->set_green(c.green);
		color_serialize.mutable_rgb()->set_blue(c.blue);
	}
	else if (std::holds_alternative<svg::Rgba>(color)) {
		
		svg::Rgba c = std::get<svg::Rgba>(color);
		color_serialize.mutable_rgba()->set_red(c.red);
		color_serialize.mutable_rgba()->set_green(c.green);
		color_serialize.mutable_rgba()->set_blue(c.blue);
		color_serialize.mutable_rgba()->set_opacity(c.opacity);
	}
	return color_serialize;
}

render_settings_serialize::RenderSettings SerializeRenderSettings(const RenderSettings& settings) {
    render_settings_serialize::RenderSettings render_settings;
    
    render_settings.set_width(settings.width);
    render_settings.set_height(settings.height);
    render_settings.set_padding(settings.padding);
    render_settings.set_line_width(settings.line_width);
    render_settings.set_stop_radius(settings.stop_radius);
    render_settings.set_bus_label_font_size(settings.bus_label_font_size);
    render_settings.set_stop_label_font_size(settings.stop_label_font_size);
    render_settings.set_underlayer_width(settings.underlayer_width);
    
    for (const auto& entry: settings.bus_label_offset) {
        render_settings.add_bus_label_offset(entry);
    }
    
    for (const auto& entry: settings.stop_label_offset) {
        render_settings.add_stop_label_offset(entry);
    }
    
    *render_settings.mutable_underlayer_color() = std::move(TransformToSerializeColor(settings.underlayer_color));

    for (const auto& color: settings.color_palette) {
        *render_settings.add_color_palette() = std::move(TransformToSerializeColor(color));
    }
    
    return render_settings;
}

void SerializeTransportCatalogue(std::ostream& out, const transport_catalogue::TransportCatalogue& db,
                                  const RenderSettings& render_settings, const RoutingSettings& routing_settings) {
    transport_catalogue_serialize::TransportCatalogue catalogue_serialize;
    
    auto stops = db.GetStops();
    for(const auto& stop: stops) {
        transport_catalogue_serialize::Coordinates coordinates;
        coordinates.set_lat(stop.coordinates.lat);
        coordinates.set_lng(stop.coordinates.lng);

        transport_catalogue_serialize::Stop stop_serialize;
        stop_serialize.set_name(stop.name);
        *stop_serialize.mutable_coordinates() = std::move(coordinates);

        *catalogue_serialize.add_stop() = std::move(stop_serialize);
    }

    auto buses = db.GetBuses();
    for(const auto& bus: buses) {
        transport_catalogue_serialize::Bus bus_serialize;
        bus_serialize.set_name(bus.name);
        bus_serialize.set_is_roundtrip(bus.is_roundtrip);

        for (auto stop: bus.route) {
            *bus_serialize.add_stop_name() = stop->name;
        }

        *catalogue_serialize.add_bus() = std::move(bus_serialize);
    }

    auto road_distances = db.GetRoadDistances();
    for (const auto [stop_ptr_pair, distance]: road_distances ) {
        transport_catalogue_serialize::RoadDistance road_distance_serialize;
        road_distance_serialize.set_from(stop_ptr_pair.first->name);
        road_distance_serialize.set_to(stop_ptr_pair.second->name);
        road_distance_serialize.set_distance(distance);

        *catalogue_serialize.add_road_distance() = std::move(road_distance_serialize);
    }

    auto settings_serialize = SerializeRenderSettings(render_settings);
    *catalogue_serialize.mutable_render_settings() = std::move(settings_serialize);
    
    auto routing_settings_serialize = SerializeRoutingSettings(routing_settings);
    *catalogue_serialize.mutable_routing_settings() = std::move(routing_settings_serialize);
    
    catalogue_serialize.SerializeToOstream(&out);
}

svg::Color TransformToSvgColor(const svg_serialize::Color& color_serialize) {
    svg::Color color;
    
    if (color_serialize.has_string_color()) {
        color = color_serialize.string_color().color();
    } else if (color_serialize.has_rgb()) {
        svg::Rgb rgb;
        rgb.red = color_serialize.rgb().red();
        rgb.green = color_serialize.rgb().green();
        rgb.blue = color_serialize.rgb().blue();
        color = rgb;
    } else if (color_serialize.has_rgba()) {
        svg::Rgba rgba;
        rgba.red = color_serialize.rgba().red();
        rgba.green = color_serialize.rgba().green();
        rgba.blue = color_serialize.rgba().blue();
        rgba.opacity = color_serialize.rgba().opacity();
        color = rgba;
    }
    
    return color;
}

void DeserializeRenderSettings(const render_settings_serialize::RenderSettings& settings_serialize,
                                            RenderSettings& settings) {
    settings.width = settings_serialize.width();
    settings.height = settings_serialize.height();
    settings.padding = settings_serialize.padding();
    settings.line_width = settings_serialize.line_width();
    settings.stop_radius = settings_serialize.stop_radius();
    settings.bus_label_font_size = settings_serialize.bus_label_font_size(); 
    settings.stop_label_font_size = settings_serialize.stop_label_font_size();
    settings.underlayer_width = settings_serialize.underlayer_width();
    settings.underlayer_color = TransformToSvgColor(settings_serialize.underlayer_color());
    
    for (int i = 0; i != settings_serialize.bus_label_offset_size(); ++i) {
        settings.bus_label_offset.push_back(settings_serialize.bus_label_offset(i));
    }
    
    for (int i = 0; i != settings_serialize.stop_label_offset_size(); ++i) {
        settings.stop_label_offset.push_back(settings_serialize.stop_label_offset(i));
    }
    
    for (int i = 0; i != settings_serialize.color_palette_size(); ++i) {
        settings.color_palette.push_back(TransformToSvgColor(settings_serialize.color_palette(i)));
    }
} 

void DeserializeTransportCatalogue(std::istream& in, transport_catalogue::TransportCatalogue& transport_catalogue,
                                   RenderSettings& render_settings, RoutingSettings& routing_settings) {
    transport_catalogue_serialize::TransportCatalogue catalogue_serialize;
    catalogue_serialize.ParseFromIstream(&in);

    for (int i = 0; i != catalogue_serialize.stop_size(); ++i) {
        auto stop_deserialized = catalogue_serialize.stop(i);
        transport_catalogue.AddStop(stop_deserialized.name(), 
                                    stop_deserialized.coordinates().lat(), 
                                    stop_deserialized.coordinates().lng());
    }

    for (int i = 0; i != catalogue_serialize.bus_size(); ++i) {
        auto bus_deserialized = catalogue_serialize.bus(i);

        std::vector<std::string> stop_names;
        for (int j = 0; j != bus_deserialized.stop_name_size(); ++j) {
            stop_names.push_back(bus_deserialized.stop_name(j));
        }

        transport_catalogue.AddBus(bus_deserialized.name(), stop_names, bus_deserialized.is_roundtrip());
    }

    for (int i = 0; i != catalogue_serialize.road_distance_size(); ++i) {
        auto road_distance_deserialized = catalogue_serialize.road_distance(i);
        
        auto from = transport_catalogue.FindStop(road_distance_deserialized.from());
        auto to = transport_catalogue.FindStop(road_distance_deserialized.to());

        transport_catalogue.SetDistanceBetweenStops(from, to, road_distance_deserialized.distance());
    }

    DeserializeRenderSettings(catalogue_serialize.render_settings(), render_settings);
    DeserializeRoutingSettings(catalogue_serialize.routing_settings(), routing_settings);
}

transport_router_serialize::RoutingSettings SerializeRoutingSettings(const RoutingSettings& settings) {
    transport_router_serialize::RoutingSettings settings_serialize;
    
    settings_serialize.set_bus_wait_time(settings.bus_wait_time);
    settings_serialize.set_bus_velocity(settings.bus_velocity);
    
    return settings_serialize;
}

void DeserializeRoutingSettings(const transport_router_serialize::RoutingSettings& settings_serialize, RoutingSettings& settings) {
    settings.bus_wait_time = settings_serialize.bus_wait_time();
    settings.bus_velocity = settings_serialize.bus_velocity();
}

