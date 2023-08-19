#include "svg.h"

#include <iostream>
#include <cstdint>

namespace svg {

using namespace std::literals;

Rgb::Rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) 
    : red(r)
    , green(g)
    , blue(b) {
}
  
Rgba::Rgba(std::uint8_t r, std::uint8_t g , std::uint8_t b, double o)
    : red(r)
    , green(g)
    , blue(b)
    , opacity(o) {
}
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}
    
std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
    }
    return out;
}
    
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
    }
    return out;
}
            
// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}
   
// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    if (!points_.empty()) {
        out << points_[0].x << ","s << points_[0].y;
        for (size_t i = 1; i != points_.size(); ++i) {
            out << " "sv << points_[i].x << ","sv << points_[i].y;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << " />"sv;
}

// ---------- Text ------------------
    
Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    for (const char& c: data) {
        switch (c) {
            case '"':
                data_ += "&quot;"s;
                break;
            case '\'':
                data_ += "&apos;"s;
                break;
            case '<':
                data_ += "&lt;"s;
                break;
            case '>':
                data_ += "&gt;"s;
                break;
            case '&':
                data_ += "&amp;"s;
                break;
            default:
                data_ += c;
        }
    }
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv;
    out << "x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }
    if (!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }
    RenderAttrs(out);
    out << ">"sv << data_ << "</text>"sv;
}
   
// ---------- Document ------------------
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }
    
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext ctx(std::cout, 2, 2);
    for (size_t i = 0; i != objects_.size(); ++i) {
        objects_.at(i)->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg