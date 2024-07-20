#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

   const double EPSILON = 1e-6;

   double ComputeDistance(Coordinates from, Coordinates to) {
       using namespace std;
       if (from == to) {
           return 0;
       }
       static const double dr = 3.1415926535 / 180.;
       static const int earth_radius = 6371000;
       return acos(sin(from.lat * dr) * sin(to.lat * dr)
           + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
           * earth_radius;
   }

   bool Coordinates::operator==(const Coordinates& other) const {
       return ((std::abs(lat - other.lat) < EPSILON) && (std::abs(lng - other.lng) < EPSILON));
   }

   bool Coordinates::operator!=(const Coordinates& other) const {
       return !(*this == other);
   }

   std::size_t CoordinatesHash::operator()(const Coordinates& coordinate) const {
       size_t latitude = (coordinate.lat < 0) ? static_cast<size_t>(std::abs(coordinate.lat) * 1'000'000)
           : static_cast<size_t>(coordinate.lat * 1'000'000) + 100'000'000;
       size_t longitude = (coordinate.lng < 0) ? static_cast<size_t>(std::abs(coordinate.lng) * 1'000'000)
           : static_cast<size_t>(coordinate.lng * 1'000'000) + 200'000'000;
       return (latitude * 1'000'000'000) + longitude;
   }

}  // namespace geo