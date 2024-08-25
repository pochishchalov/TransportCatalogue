#include "request_handler.h"

#include <vector>

namespace handler {

	using namespace std;

	const renderer::BusesContainer RequestHandler::GetBusesPtr() const {
		return catalogue_.GetUniqueBuses();
	}

	const renderer::StopsContainer RequestHandler::GetStopsPtr() const {
		return catalogue_.GetUniqueStops();
	}

	const optional<RequestHandler::StopInformation> RequestHandler::GetStopInformation(
		const string_view stop_name) const
	{
		return catalogue_.GetStopInformation(stop_name);
	}

	const optional<RequestHandler::RouteInformation> RequestHandler::GetRouteInformation(
		const string_view bus_name) const 
	{
		return catalogue_.GetRouteInformation(bus_name);
	}

	svg::Document RequestHandler::RenderMap() const{
		return renderer_.Render(GetBusesPtr(), GetStopsPtr());
	}

	const optional<RequestHandler::RouterInformation> RequestHandler::GetRouterInfo(string_view from,
		string_view to) const
	{
		return router_.GetRouteInfo(from, to);
	}

} // namespace handler