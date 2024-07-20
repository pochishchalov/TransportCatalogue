#include "domain.h"

namespace domain {

	bool StopPtrNameCompare::operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->name < rhs->name;
	}

	bool BusPtrNameCompare::operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}

} // namespace domain