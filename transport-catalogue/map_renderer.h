#pragma once
#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <vector>

namespace renderer {

    // Для хранения уникальных маршрутов в лексиграфическом порядке по названию
    using BusesContainer = std::vector<const domain::Bus*>;

    // Для хранения уникальных остановок в лексиграфическом порядке по названию
    using StopsContainer = std::vector<const domain::Stop*>;

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:

        SphereProjector() = default;

        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
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
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_ = 0.0;
        double min_lon_ = 0.0;
        double max_lat_ = 0.0;
        double zoom_coeff_ = 0.0;
    };

    // Структура для хранения настроек MapRenderer
    struct MapRendererSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        int stop_label_font_size = 0;
        svg::Point bus_label_offset;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;
    };


    class MapRenderer {
    public:

        MapRenderer(MapRendererSettings settings)
            :settings_(std::move(settings))
        {
        }

        // Возвращает svg::Document с визуализиацией карты маршрутов 
        svg::Document Render(const BusesContainer& buses) const;

    private:
        MapRendererSettings settings_;

        //------------------------------------------------------
        
                    // Функции отрисовки

        // Отрисовывает линии маршрутов Bus
        void RenderLines(const BusesContainer& buses, const SphereProjector& projector, svg::Document& doc) const;

        // Отрисовывает текст названия маршрутов Bus
        void RenderBusText(const BusesContainer& buses, const SphereProjector& projector, svg::Document& doc) const;

        // Отрисовывает символы остановок
        void RenderStopSymbols(const StopsContainer& stops, const SphereProjector& projector, svg::Document& doc) const;

        // Отрисовывает названия остановок
        void RenderStopsText(const StopsContainer& stops, const SphereProjector& projector, svg::Document& doc) const;

        //------------------------------------------------------
        
            // Вспомогательные функции отрисовки

        // Возвращает линию маршрута Bus
        const svg::Polyline GetPolylineRoute(const domain::Bus* bus, const SphereProjector& projector) const;

        // Возвращает "базовый" текст названия маршрута Bus
        const svg::Text GetBusBaseText(const std::string& name) const;

        // Добавляет текст, а именно текстовую подложку и сам основной текст
        void AddText(const svg::Text& base_text, svg::Document& doc, const svg::Color& color, const svg::Point& point) const;

        // Получаем "базовый" текст названия остановки Stop
        const svg::Text GetStopBaseText(const std::string& name) const;

        //------------------------------------------------------

                // Вспомогательные функции
        
        // Возвращает набор всех координат всех маршрутов Bus
        const std::vector<geo::Coordinates> GetAllCoordinates(const StopsContainer& stops) const;

        // Возвращает SphereProjector постороенный из BusesContainer
        const SphereProjector CreateProjector(const StopsContainer& stops) const;

        // Возвращает набор (set) остановок отсортированных в лексиграфическом порядке по названию
        const StopsContainer GetStopsContainer(const BusesContainer& buses) const;

    };

} // namespace renderer