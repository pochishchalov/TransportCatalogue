#pragma once

#include <cmath>

#include <iostream>

namespace geo {

    struct Coordinates {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
     };

    double ComputeDistance(Coordinates from, Coordinates to);

    struct CoordinatesHash {
        std::size_t operator()(const Coordinates& coordinate) const;
    };
}