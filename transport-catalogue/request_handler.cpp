#include "request_handler.h"

#include <set>

namespace handler {

	using namespace std;

	// Для хранения уникальных маршрутов в лексиграфическом порядке по названию
	using BusesContainer = std::set<const domain::Bus*, domain::BusPtrNameCompare>;

	const BusesContainer RequestHandler::GetBusesPtr() const {
		std::set<const domain::Bus*, domain::BusPtrNameCompare> result;
		const auto& buses = catalogue_.GetBuses();
		for (const auto& bus : buses) {
			result.insert(bus.second);
		}
		return result;
	}

	svg::Document RequestHandler::RenderMap() const {

		return renderer_.Render(GetBusesPtr());
	}

} // namespace handler