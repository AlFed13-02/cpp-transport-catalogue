#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {
    
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};
    
struct Rgb {
    std::uint8_t red = 0, green = 0, blue = 0;
    
    Rgb() = default;
    
    Rgb(std::uint8_t r, std::uint8_t g , std::uint8_t b);
};
   
struct Rgba {
    std::uint8_t red = 0, green = 0, blue = 0;
    double opacity = 1.;
    
    Rgba() = default;
    
    Rgba(std::uint8_t r, std::uint8_t g , std::uint8_t b, double o);
};
    
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    
    
inline const Color NoneColor;
    
struct ColorPrinter {
    std::ostream& out;
    
    void operator()(std::monostate) const {
        using namespace std::literals;
        out << "none"sv;
    }
    
    void operator()(std::string color) const{
        using namespace std::literals;
        out << color;
    }

    void operator()(Rgb color) const {
        using namespace std::literals;
        out << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
    }
    
    void operator()(Rgba color) const {
        using namespace std::literals;
        out << "rgba("s << static_cast<int>(color.red) << ","s << static_cast<int>(color.green) << ","s << static_cast<int>(color.blue) <<","s << color.opacity << ")"s;
    }
};
    
std::ostream& operator<<(std::ostream& out, const Color& color);

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};
    
class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        auto ptr = std::make_unique<Obj>(std::move(obj));
        AddPtr(std::move(ptr));
    }  
    
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};
    
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void Draw(ObjectContainer& object_container) const = 0;
}; 
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
    
std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            width_ = std::move(width);
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\"" << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\"" << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\"" << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);
    
private:
    void RenderObject(const RenderContext& context) const override;
    
    std::vector<Point> points_;
   };

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    
    Point position_ = {0., 0.};
    Point offset_ = {0., 0.};
    std::uint32_t font_size_ = 1;
    std::string font_weight_;
    std::string font_family_;
    std::string data_;
    
};
    

class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    // void Add(???);
    
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg