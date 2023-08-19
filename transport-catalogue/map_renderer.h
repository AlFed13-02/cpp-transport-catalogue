#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <utility>

inline const double EPSILON = 1e-6;
bool IsZero(double value);
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
*/

struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius; 
    int bus_label_font_size;
    std::vector<double> bus_label_offset;
    int stop_label_font_size;
    std::vector<double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);
    
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

class MapRenderer {
public:
    template <typename PointInputIt>
    MapRenderer(PointInputIt points_begin, PointInputIt points_end, const RenderSettings& settings);
    
    svg::Document Render(const domain::MapStat& map_stat) const;
    
private:
    SphereProjector projector_;
    RenderSettings settings_;
    
    
    void RenderRouteLines(svg::Document& doc, const domain::MapStat& map_stat) const;
    void RenderText(svg::Document& doc, const geo::Coordinates& coordinates, 
                    const std::string& data, const svg::Color& fill_color,
                    svg::Point offset, int font_size, std::string font_weight) const;
    void RenderRouteNames(svg::Document& doc, const domain::MapStat& map_stat) const;
    void RenderStopCircles(svg::Document& doc, const domain::MapStat& map_stat) const;
    void RenderStopNames(svg::Document& doc, const domain::MapStat& map_stat) const;
    
};

template <typename PointInputIt>
MapRenderer::MapRenderer(PointInputIt points_begin, PointInputIt points_end, const RenderSettings& settings)
    : projector_(points_begin, points_end, settings.width, settings.height, settings.padding)
    , settings_(std::move(settings)) {}