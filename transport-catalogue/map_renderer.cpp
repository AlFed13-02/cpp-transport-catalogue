#include "map_renderer.h"
#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <utility>
#include <tuple>

using namespace std::literals;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
// points_begin и points_end задают начало и конец интервала элементов geo::Coordinate

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

svg::Document MapRenderer::Render(const domain::MapStat& map_stat) const {
    svg::Document doc;
    RenderRouteLines(doc, map_stat);
    RenderRouteNames(doc, map_stat);
    RenderStopCircles(doc, map_stat);
    RenderStopNames(doc, map_stat);
    return doc;
}
    
    
void MapRenderer::RenderRouteLines(svg::Document& doc, const domain::MapStat& map_stat) const {
    for (auto  [i, it] = std::tuple{0, map_stat.buses.begin()}; it != map_stat.buses.end(); ++i, ++it) {
        svg::Polyline polyline;
        polyline.SetFillColor(svg::NoneColor).SetStrokeWidth(settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeColor(settings_.color_palette[i % settings_.color_palette.size()]);
        
        for (auto stop: (*it)->route) {
            polyline.AddPoint(projector_(stop->coordinates));
        }
        doc.Add(std::move(polyline));
    }
}

void MapRenderer::RenderText(svg::Document& doc, const geo::Coordinates& coordinates, 
                             const std::string& data, const svg::Color& fill_color,
                             svg::Point offset, int font_size, std::string font_weight) const {
    svg::Text underlayer;
    underlayer.SetPosition(projector_(coordinates))
              .SetOffset(offset)
              .SetFontSize(font_size).SetFontFamily("Verdana"s).SetData(data);
    
    if (!font_weight.empty()) {
        underlayer.SetFontWeight(font_weight);
    }
        
    svg::Text text = underlayer;
        
    underlayer.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
              .SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
              .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
    text.SetFillColor(fill_color);
        
    doc.Add(std::move(underlayer));
    doc.Add(std::move(text));
}

void MapRenderer::RenderRouteNames(svg::Document& doc, const domain::MapStat& map_stat) const {
    for (auto  [i, it] = std::tuple{0, map_stat.buses.begin()}; it != map_stat.buses.end(); ++i, ++it) {
        auto fill_color = settings_.color_palette[i % settings_.color_palette.size()];
        auto data = (*it)->name;
        auto offset = svg::Point{settings_.bus_label_offset[0], settings_.bus_label_offset[1]};
        auto font_size = settings_.bus_label_font_size;
        auto font_weight = "bold"s;
        RenderText(doc, (*it)->route[0]->coordinates, data, fill_color, offset, font_size, font_weight);
        if (!(*it)->is_roundtrip && (*it)->route[((*it)->route.size() - 1) / 2] != (*it)->route[0]) {
            RenderText(doc, (*it)->route[((*it)->route.size() - 1) / 2]->coordinates, data, fill_color, offset, font_size, font_weight);
        }
    }
}

void MapRenderer::RenderStopCircles(svg::Document& doc, const domain::MapStat& map_stat) const {
    for (const auto& stop: map_stat.stops) {
        svg::Circle circle;
        circle.SetCenter(projector_(stop->coordinates)).SetRadius(settings_.stop_radius)
              .SetFillColor("white"s);
        doc.Add(std::move(circle));                 
    }
}

void MapRenderer::RenderStopNames(svg::Document& doc, const domain::MapStat& map_stat) const {
    for ( const auto& stop: map_stat.stops) {
        svg::Color fill_color = "black"s;
        auto data = stop->name;
        auto offset = svg::Point{settings_.stop_label_offset[0], settings_.stop_label_offset[1]};
        auto font_size = settings_.stop_label_font_size;
        auto font_weight = ""s;
        RenderText(doc, stop->coordinates, data, fill_color, offset, font_size, font_weight);
    }
}