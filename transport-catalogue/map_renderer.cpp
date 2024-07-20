#include "map_renderer.h"

#include <set>
#include <unordered_set>
#include <iostream>

namespace renderer {

	using namespace std::literals;

	// Для хранения уникальных маршрутов в лексиграфическом порядке по названию
	using AllBusesPtr = std::set<const domain::Bus*, domain::BusPtrNameCompare>;

	// Для хранения уникальных остановок в лексиграфическом порядке по названию
	using AllStopsPtr = std::set<const domain::Stop*, domain::StopPtrNameCompare>;

	// Возвращает набор всех координат всех маршрутов Bus
	std::unordered_set<geo::Coordinates, geo::CoordinatesHash> GetAllCoordinates(const AllBusesPtr& buses) {
		std::unordered_set<geo::Coordinates, geo::CoordinatesHash> result;
		for (const auto& bus : buses) {
			for (const auto& bus_stop : bus->bus_stops) {
				result.insert(bus_stop->coordinates);
			}
		}
		return result;
	}

	// Возвращает SphereProjector постороенный
	SphereProjector GetSphereProjector(const AllBusesPtr& buses, const MapRendererSettings& settings) {

		// Точки, подлежащие проецированию
		const auto coordinates = GetAllCoordinates(buses);

		return {
			coordinates.begin(), coordinates.end(),
			settings.width, settings.height, settings.padding
		};
	}

	// Возвращает линию маршрута Bus
	svg::Polyline GetPolylineRoute(const domain::Bus* bus, const SphereProjector& projector) {
		svg::Polyline polyline;
		for (const auto& bus_stop : bus->bus_stops) {
			polyline.AddPoint(projector(bus_stop->coordinates));
		}
		return polyline;
	}

	// Отрисовывает линии маршрутов Bus
	void RenderLines(const AllBusesPtr& buses, const MapRendererSettings& settings,
		const SphereProjector& projector, svg::Document& doc) {

		// Итератор ссылающийся на цвет линии маршрута
		auto color_iter = settings.color_palette.begin();

		// Отрисовываем маршруты
		for (const auto& bus : buses) {

			if (bus->bus_stops.empty()) {
				continue;
			}
			svg::Polyline polyline = GetPolylineRoute(bus, projector);

			// Добавляем Polyline в Document с настройками из settings_
			doc.Add(polyline.SetStrokeColor(*color_iter)
				.SetFillColor(svg::NoneColor)
				.SetStrokeWidth(settings.line_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

			(color_iter + 1 == settings.color_palette.end()) ? color_iter = settings.color_palette.begin() : ++color_iter;
		}
	}

	// Получаем "базовый" текст названия маршрута Bus
	svg::Text GetBusBaseText(const std::string& name, const MapRendererSettings& settings) {
		return svg::Text().SetOffset(settings.bus_label_offset)
						.SetFontSize(settings.bus_label_font_size)
						.SetFontFamily("Verdana"s)
						.SetFontWeight("bold"s)
						.SetData(name);
	}

	// Добавляет текст, а именно текстовую подложку и сам основной текст
	void AddText(const svg::Text& base_text, svg::Document& doc, const SphereProjector& projector,
		const MapRendererSettings& settings, const svg::Color& color, const geo::Coordinates& coord,
		const svg::StrokeLineCap line_cap, const svg::StrokeLineJoin line_join)
	{
		const svg::Point point_coord = projector(coord);

		// Добавляем подложку
		doc.Add(svg::Text{ base_text }.SetPosition(point_coord)
									.SetFillColor(settings.underlayer_color)
									.SetStrokeColor(settings.underlayer_color)
									.SetStrokeWidth(settings.underlayer_width)
									.SetStrokeLineCap(line_cap)
									.SetStrokeLineJoin(line_join));

		// Добавляем основную надпись
		doc.Add(svg::Text{ base_text }.SetPosition(point_coord)
									.SetFillColor(color));
	}

	// Отрисовывает текст названия маршрутов Bus
	void RenderBusText(const AllBusesPtr& buses, const MapRendererSettings& settings,
		const SphereProjector& projector, svg::Document& doc) {

		// Итератор ссылающийся на цвет линии маршрута
		auto color_iter = settings.color_palette.begin();

		// Отрисовываем названия маршрутов
		for (const auto& bus : buses) {

			if (bus->bus_stops.empty()) {
				continue;
			}

			// Получаем базовый Text названия маршрута
			svg::Text bus_name_text = GetBusBaseText(bus->name, settings);

			// Добавляем название маршрута в координаты конечной остановки
			AddText(bus_name_text, doc, projector, settings, *color_iter, bus->bus_stops[0]->coordinates,
				svg::StrokeLineCap::ROUND, svg::StrokeLineJoin::ROUND);

			// Если маршрут не кольцевой - добавляем название маршрута у второй конечной остановки
			if (!bus->is_roundtrip) {
				const auto& second_end_station = bus->bus_stops[bus->bus_stops.size() / 2];
				if (second_end_station->name != bus->bus_stops[0]->name) {
					AddText(bus_name_text, doc, projector, settings, *color_iter, second_end_station->coordinates, 
						svg::StrokeLineCap::ROUND, svg::StrokeLineJoin::ROUND);
				}
			}
			(color_iter + 1 == settings.color_palette.end()) ? color_iter = settings.color_palette.begin() : ++color_iter;
		}
	}

	// Возвращает набор (set) остановок отсортированных в лексиграфическом порядке по названию
	const AllStopsPtr GetAllStops(const AllBusesPtr& buses) {
		AllStopsPtr result;
		for (const auto& bus : buses) {
			result.insert(bus->bus_stops.begin(), bus->bus_stops.end());
		}
		return result;
	}

	// Отрисовывает символы остановок
	void RenderStopSymbols(const AllStopsPtr& stops, const MapRendererSettings& settings,
		const SphereProjector& projector, svg::Document& doc) {
		 
		for (const auto& stop : stops) {
			doc.Add(svg::Circle().SetCenter(projector(stop->coordinates))
								.SetRadius(settings.stop_radius)
								.SetFillColor("white"s));
		}
	}

	// Получаем "базовый" текст названия остановки Stop
	svg::Text GetStopBaseText(const std::string& name, const MapRendererSettings& settings) {
		return svg::Text().SetOffset(settings.stop_label_offset)
			.SetFontSize(settings.stop_label_font_size)
			.SetFontFamily("Verdana"s)
			.SetData(name);
	}

	// Отрисовывает названия остановок
	void RenderStopsText(const AllStopsPtr& stops, const MapRendererSettings& settings,
		const SphereProjector& projector, svg::Document& doc) {

		// Отрисовываем названия остановок
		for (const auto& stop : stops) {

			// Получаем базовый Text названия остановки
			svg::Text stop_name_text = GetStopBaseText(stop->name, settings);

			// Добавляем название остановки
			AddText(stop_name_text, doc, projector, settings, svg::Color("black"s), stop->coordinates,
				svg::StrokeLineCap::ROUND, svg::StrokeLineJoin::ROUND);
		}
	}

	svg::Document MapRenderer::Render(const AllBusesPtr& buses) const {

		svg::Document doc;

		// Создаём проектор сферических координат на карту
		const SphereProjector projector = GetSphereProjector(buses, settings_);

		// Отрисовываем линии маршрутов
		RenderLines(buses, settings_, projector, doc);

		// Отрисовываем названия маршрутов
		RenderBusText(buses, settings_, projector, doc);

		// Получаем уникальные остановки в лексиграфическом порядке
		const auto stops = GetAllStops(buses);

		// Отрисовываем символы остановок
		RenderStopSymbols(stops, settings_, projector, doc);

		// Отрисовываем названия остановок
		RenderStopsText(stops, settings_, projector, doc);

		return doc;

	}

} // namespace renderer