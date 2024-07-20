#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& cap) {
        switch (cap)
        {
        case StrokeLineCap::BUTT:
            os << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            os << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"sv;
            break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& join) {
        switch (join)
        {
        case StrokeLineJoin::ARCS:
            os << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"sv;
            break;
        }
        return os;
    }

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }

    void ColorPrinter::operator()(const std::string& color) const {
        out << color;
    }

    void ColorPrinter::operator()(const Rgb& rgb) const {
        out << "rgb("sv << static_cast<int>(rgb.red) << ','
            << static_cast<int>(rgb.green) << ','
            << static_cast<int>(rgb.blue) << ')';
    }

    void ColorPrinter::operator()(const Rgba& rgba) const {
        out << "rgba("sv << static_cast<int>(rgba.red) << ','
            << static_cast<int>(rgba.green) << ','
            << static_cast<int>(rgba.blue) << ','
            << rgba.opacity << ')';
    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {

        std::visit(ColorPrinter{ os }, color);
        return os;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        bool is_first = true;
        out << "<polyline points=\""sv;
        for (const auto& point : points_) {
            if (!is_first) {
                out << ' ';
            }
            out << point.x << ',' << point.y;
            is_first = false;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text -------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = std::move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    std::string Text::RenderText(const std::string& text) const {
        std::string result;
        for (const char ch : text) {
            switch (ch)
            {
            case '"':
                result += "&quot;"sv;
                break;
            case '\'':
                result += "&apos;"sv;
                break;
            case '<':
                result += "&lt;"sv;
                break;
            case '>':
                result += "&gt;"sv;
                break;
            case '&':
                result += "&amp;"sv;
                break;
            default:
                result += ch;
                break;
            }
        }
        return result;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv << "x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(context.out);
        out << ">"sv << RenderText(data_);
        out << "</text>"sv;
    }

    // ---------- Document ---------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        for (const auto& obj : objects_) {
            obj.get()->Render({ out, 0, 2 });
        }
        out << "</svg>";
    }

}  // namespace svg