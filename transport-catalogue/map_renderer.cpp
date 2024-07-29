#include "map_renderer.h"

#include <vector>

namespace renderer {

	using namespace std::literals;

	const std::vector<geo::Coordinates> MapRenderer::GetAllCoordinates(const StopsContainer& stops) const {
		
		std::vector<geo::Coordinates> result;
		result.reserve(stops.size());
		for (const auto& stop : stops) {
			result.push_back(stop->coordinates);
		}

		return result;
	}

	const SphereProjector MapRenderer::CreateProjector(const StopsContainer& stops) const{

		// Точки, подлежащие проецированию
		const auto coordinates = GetAllCoordinates(stops);

		return {
			coordinates.begin(), coordinates.end(),
			settings_.width, settings_.height, settings_.padding
		};
	}

	const svg::Polyline MapRenderer::GetPolylineRoute(const domain::Bus* bus, const SphereProjector& projector) const {
		svg::Polyline polyline;
		for (const auto& bus_stop : bus->bus_stops) {
			polyline.AddPoint(projector(bus_stop->coordinates));
		}
		return polyline;
	}

	void MapRenderer::RenderLines(const BusesContainer& buses, const SphereProjector& projector, svg::Document& doc) const {

		// Итератор ссылающийся на цвет линии маршрута
		auto color_iter = settings_.color_palette.begin();

		// Отрисовываем маршруты
		for (const auto& bus : buses) {

			if (bus->bus_stops.empty()) {
				continue;
			}
			svg::Polyline polyline = GetPolylineRoute(bus, projector);

			// Добавляем Polyline в Document с настройками из settings_
			doc.Add(polyline.SetStrokeColor(*color_iter)
				.SetFillColor(svg::NoneColor)
				.SetStrokeWidth(settings_.line_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

			(color_iter + 1 == settings_.color_palette.end()) ? color_iter = settings_.color_palette.begin() : ++color_iter;
		}
	}

	const svg::Text MapRenderer::GetBusBaseText(const std::string& name) const {
		return svg::Text().SetOffset(settings_.bus_label_offset)
						.SetFontSize(settings_.bus_label_font_size)
						.SetFontFamily("Verdana"s)
						.SetFontWeight("bold"s)
						.SetData(name);
	}

	void MapRenderer::AddText(const svg::Text& base_text, svg::Document& doc, const svg::Color& color,
		const svg::Point& point) const
	{

		// Добавляем подложку
		doc.Add(svg::Text{ base_text }.SetPosition(point)
									.SetFillColor(settings_.underlayer_color)
									.SetStrokeColor(settings_.underlayer_color)
									.SetStrokeWidth(settings_.underlayer_width)
									.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
									.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

		// Добавляем основную надпись
		doc.Add(svg::Text{ base_text }.SetPosition(point)
									.SetFillColor(color));
	}

	void MapRenderer::RenderBusText(const BusesContainer& buses, const SphereProjector& projector, svg::Document& doc) const {

		// Итератор ссылающийся на цвет линии маршрута
		auto color_iter = settings_.color_palette.begin();

		// Отрисовываем названия маршрутов
		for (const auto& bus : buses) {

			if (bus->bus_stops.empty()) {
				continue;
			}

			// Получаем базовый Text названия маршрута
			svg::Text bus_name_text = GetBusBaseText(bus->name);

			// Добавляем название маршрута в координаты конечной остановки
			AddText(bus_name_text, doc, *color_iter, projector(bus->bus_stops[0]->coordinates));

			// Если маршрут не кольцевой - добавляем название маршрута у второй конечной остановки
			if (!bus->is_roundtrip) {
				const auto& second_end_station = bus->bus_stops[bus->bus_stops.size() / 2];
				if (second_end_station->name != bus->bus_stops[0]->name) {
					AddText(bus_name_text, doc, *color_iter, projector(second_end_station->coordinates));
				}
			}
			(color_iter + 1 == settings_.color_palette.end()) ? color_iter = settings_.color_palette.begin() : ++color_iter;
		}
	}

	const StopsContainer MapRenderer::GetStopsContainer(const BusesContainer& buses) const {
		StopsContainer result;
		for (const auto& bus : buses) {
			result.insert(result.end(), bus->bus_stops.begin(), bus->bus_stops.end());
		}
		std::sort(result.begin(), result.end(), [](const domain::Stop* lhs, const domain::Stop* rhs) {
			return lhs->name < rhs->name;
			});
		result.erase(std::unique(result.begin(), result.end()), result.end());
		return result;
	}

	void MapRenderer::RenderStopSymbols(const StopsContainer& stops, const SphereProjector& projector, svg::Document& doc) const {
		 
		for (const auto& stop : stops) {
			doc.Add(svg::Circle().SetCenter(projector(stop->coordinates))
								.SetRadius(settings_.stop_radius)
								.SetFillColor("white"s));
		}
	}

	const svg::Text MapRenderer::GetStopBaseText(const std::string& name) const {
		return svg::Text().SetOffset(settings_.stop_label_offset)
			.SetFontSize(settings_.stop_label_font_size)
			.SetFontFamily("Verdana"s)
			.SetData(name);
	}

	void MapRenderer::RenderStopsText(const StopsContainer& stops, const SphereProjector& projector, svg::Document& doc) const {

		// Отрисовываем названия остановок
		for (const auto& stop : stops) {

			// Получаем базовый Text названия остановки
			svg::Text stop_name_text = GetStopBaseText(stop->name);

			// Добавляем название остановки
			AddText(stop_name_text, doc, svg::Color("black"s), projector(stop->coordinates));
		}
	}

	svg::Document MapRenderer::Render(const BusesContainer& buses) const{

		svg::Document doc;

		// Получаем уникальные остановки в лексиграфическом порядке
		const auto stops = GetStopsContainer(buses);
		
		// Создаём проектор сферических координат на карту и добавляем его в MapRenderer
		const SphereProjector projector = CreateProjector(stops);
		
		// Отрисовываем линии маршрутов
		RenderLines(buses, projector, doc);

		// Отрисовываем названия маршрутов
		RenderBusText(buses, projector, doc);

		// Отрисовываем символы остановок
		RenderStopSymbols(stops, projector, doc);

		// Отрисовываем названия остановок
		RenderStopsText(stops, projector, doc);

		return doc;

	}

} // namespace renderer