#include "json_reader.h"
#include "json_builder.h"

#include <iostream>

namespace reader {

	using namespace std::literals;

	using Commands = std::pair<std::vector<size_t>, std::vector<size_t>>;

	JsonReader::JsonReader(std::istream& input)
		:document_(json::Load(input))
	{
	}


	// Проходится по всем запросам, наполняет TransportCatalogue остановками,
	// возвращает пару векторов с позициями команд - pair<команды остановок, команды автобусов>
	Commands AddStops(const json::Array& base_requests, transport_catalogue::TransportCatalogue& catalogue) {
		std::vector<std::size_t> bus_commands_pos, stop_commands_pos;
		bus_commands_pos.reserve(base_requests.size() / 2);
		stop_commands_pos.reserve(base_requests.size() / 2);
		for (size_t i = 0; i < base_requests.size(); ++i) {
			const auto& map_request = base_requests[i].AsMap();
			const auto& type = map_request.at("type"s);
			if (type.AsString() == "Stop"s) {
				catalogue.AddStop({ map_request.at("name"s).AsString(),
				{ map_request.at("latitude"s).AsDouble(), map_request.at("longitude"s).AsDouble()} });
				stop_commands_pos.push_back(i);
			}
			else if (type.AsString() == "Bus"s) {
				bus_commands_pos.push_back(i);
			}
		}
		return { std::move(stop_commands_pos), std::move(bus_commands_pos) };
	}

	// Наполняет TransportCatalogue информацией о расстояниях между остановками
	void AddRoadDistances(const json::Array& base_requests, transport_catalogue::TransportCatalogue& catalogue,
		const std::vector<std::size_t>& stop_commands_pos) {

		for (const auto stop_command : stop_commands_pos) {
			const auto& map_request = base_requests[stop_command].AsMap();
			if (map_request.find("road_distances"s) != map_request.end()) {
				const std::string_view first_stop_name = map_request.at("name"s).AsString();
				for (const auto& [second_stop_name, distance] : map_request.at("road_distances"s).AsMap()) {
					catalogue.AddDistanceBetweenStops(first_stop_name, second_stop_name, distance.AsInt());
				}
			}
		}
	}

	// Наполняет TransportCatalogue автобусными остановками
	void AddBuses(const json::Array& base_requests, transport_catalogue::TransportCatalogue& catalogue,
		const std::vector<std::size_t>& bus_commands_pos) {

		for (const auto bus_command : bus_commands_pos) {
			const auto& map_request = base_requests[bus_command].AsMap();
			std::vector<const domain::Stop*> bus_stops;
			const auto& stops = map_request.at("stops"s).AsArray();
			for (const auto& stop_name : stops) {
				bus_stops.push_back(catalogue.GetStop(stop_name.AsString()));
			}
			const auto is_round = map_request.at("is_roundtrip"s).AsBool();
			if (!is_round) {
				for (auto iter = stops.crbegin() + 1; iter != stops.crend(); ++iter) {
					bus_stops.push_back(catalogue.GetStop(iter->AsString()));
				}
			}
			catalogue.AddBus({ map_request.at("name"s).AsString(), std::move(bus_stops), is_round });
		}
	}

	void JsonReader::AddBaseRequests(transport_catalogue::TransportCatalogue& catalogue) {
		const auto& base_requests = document_.GetRoot().AsMap().at("base_requests"s).AsArray();
		const Commands commands = AddStops(base_requests, catalogue);

		AddRoadDistances(base_requests, catalogue, commands.first);
		AddBuses(base_requests, catalogue, commands.second);
	}

	// Возвращает словарь с информацией по запросу "Bus"
	json::Node GetBusInfo(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue) {
		const auto& name = request.at("name"s).AsString();
		if (const auto info = catalogue.GetRouteInformation(name)) {
			const auto info_value = info.value();
			return json::Builder{}
						.StartDict()
						.Key("curvature"s).Value(info_value.curvature)
						.Key("request_id"s).Value(request.at("id"s).AsInt())
						.Key("route_length"s).Value(info_value.route_length)
						.Key("stop_count"s).Value(info_value.stops_count)
						.Key("unique_stop_count"s).Value(info_value.unique_stops_count)
						.EndDict()
						.Build();
		}
		else {
			return json::Builder{}
						.StartDict()
						.Key("request_id"s).Value(request.at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
						.EndDict()
						.Build();
		}
	}

	// Возвращает словарь с информацией по запросу "Stop"
	json::Node GetStopInfo(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue) {
		const auto& name = request.at("name"s).AsString();

		if (const auto info = catalogue.GetStopInformation(name)) {
			const auto info_value = info.value();
			json::Array info_vector;
			info_vector.reserve(info_value.size());
			for (const auto bus : info_value) {
				info_vector.emplace_back(static_cast<std::string>(bus));
			}
			return json::Builder{}
						.StartDict()
						.Key("buses"s).Value(std::move(info_vector))
						.Key("request_id"s).Value(request.at("id"s).AsInt())
						.EndDict()
						.Build();
		}
		else {
			return json::Builder{}
						.StartDict()
						.Key("request_id"s).Value(request.at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
						.EndDict()
						.Build();
		}
	}
	// Возвращает словарь с информацией по запросу "Map"
	json::Node GetMapInfo(const json::Dict& request, const handler::RequestHandler& handler) {

		svg::Document doc = handler.RenderMap();

		// Буфер для вывода карты маршрутов
		std::ostringstream buffer;

		doc.Render(buffer);
		
		return json::Builder{}
					.StartDict()
					.Key("request_id"s).Value(request.at("id"s).AsInt())
					.Key("map"s).Value(buffer.str())
					.EndDict()
					.Build();
	}

	json::Document JsonReader::GetInfo(const handler::RequestHandler& handler, const transport_catalogue::TransportCatalogue& catalogue) {

		const auto& stat_requests = document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
		if (stat_requests.empty()) {
			return {};
		}
		json::Array result;
		for (const auto& request : stat_requests) {
			const auto& map_request = request.AsMap();
			const auto& type = map_request.at("type"s).AsString();
			if (type == "Bus"s) {
				result.emplace_back(std::move(GetBusInfo(map_request, catalogue)));
			}
			else if (type == "Stop"s) {
				result.emplace_back(std::move(GetStopInfo(map_request, catalogue)));
			}
			else if (type == "Map"s) {
				result.emplace_back(std::move(GetMapInfo(map_request, handler)));
			}
		}
		return json::Document(std::move(result));
	}

	// Возвращает цвет(svg::Color) в виде строки или представении структур svg::Rgb или svg::Rgba 
	svg::Color GetColor(const json::Node& node) {
		if (node.IsArray()) {
			const auto& underlayer_color = node.AsArray();
			if (underlayer_color.size() == 3) {
				svg::Rgb result;
				result.red = static_cast<uint8_t>(underlayer_color[0].AsInt());
				result.green = static_cast<uint8_t>(underlayer_color[1].AsInt());
				result.blue = static_cast<uint8_t>(underlayer_color[2].AsInt());
				return result;
			}
			else if (underlayer_color.size() == 4) {
				svg::Rgba result;
				result.red = static_cast<uint8_t>(underlayer_color[0].AsInt());
				result.green = static_cast<uint8_t>(underlayer_color[1].AsInt());
				result.blue = static_cast<uint8_t>(underlayer_color[2].AsInt());
				result.opacity = underlayer_color[3].AsDouble();
				return result;
			}
		}
		else if (node.IsString()) {
			return node.AsString();
		}
		return std::monostate();
	}

	renderer::MapRendererSettings JsonReader::GetRenderSettings() const {

		const auto& render_settings = document_.GetRoot().AsMap().at("render_settings"s).AsMap();

		renderer::MapRendererSettings settings;

		settings.width = render_settings.at("width"s).AsDouble();
		settings.height = render_settings.at("height"s).AsDouble();
		settings.padding = render_settings.at("padding"s).AsDouble();
		settings.line_width = render_settings.at("line_width"s).AsDouble();
		settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();
		settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();

		const auto& bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
		settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };

		settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
		const auto& stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
		settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

		settings.underlayer_color = GetColor(render_settings.at("underlayer_color"s));

		settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();

		for (const auto& color : render_settings.at("color_palette"s).AsArray()) {
			settings.color_palette.push_back(GetColor(color));
		}

		return settings;
	}

} // namespace reader