#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0.0;
        double y = 0.0;
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
            return { out, indent_step, indent + indent_step };
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
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{ "none" };

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const;
        void operator()(const std::string& color) const;
        void operator()(const Rgb& rgb) const;
        void operator()(const Rgba& rgb) const;
    };

    std::ostream& operator<<(std::ostream& os, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& join);
    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& cap);

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
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap linecap) {
            stroke_linecap_ = linecap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin linejoin) {
            stroke_linejoin_ = linejoin;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

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
        std::string RenderText(const std::string& text) const;

        void RenderObject(const RenderContext& context) const override;

        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    /*
     * Интерфейс, представляющий контейнер SVG объектов.
     */
    class ObjectContainer {
    public:
        template <typename ObjectType>
        void Add(ObjectType object) {
            AddPtr(std::make_unique<ObjectType>(std::move(object)));
        }

        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        // Интерфейс не предполагает полиморфное удаление
        // Поэтому деструктор объявлен защищённым невиртуальным
        ~ObjectContainer() = default;
    };

    /*
     * Интерфейс объектов, которые могут быть нарисованы на любом объекте,
     * реализующем интерфейс ObjectContainer
     */
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        // Объекты Drawable могут удаляться полиморфно. Поэтому деструктор объявляем публичным
        // виртуальным
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg