#include "request_handler.h"

#include <vector>

namespace handler {

	using namespace std;

	const renderer::BusesContainer RequestHandler::GetBusesPtr() const {
		const auto& buses = catalogue_.GetBuses();

		std::vector<const domain::Bus*> result;
		result.reserve(buses.size());

		for (const auto& bus : buses) {
			result.push_back(bus.second);
		}
		std::sort(result.begin(), result.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
			return lhs->name < rhs->name;
			});
		return result;
	}

	svg::Document RequestHandler::RenderMap() const{
		
		return renderer_.Render(GetBusesPtr());
	}

} // namespace handler