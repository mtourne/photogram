/* Copyright 2014 Matthieu Tourne */

#ifndef HAVERSINE_DIST_H
#define HAVERSINE_DIST_H

#include <cmath>
#include <opencv2/flann/flann.hpp>

using namespace cvflann;

template<class ResultType>
static inline double to_radians (ResultType d) {
    return d * M_PI / 180;
}

// haversine distance to calculate the great-circle distance between
// two points – that is, the shortest distance over the earth’s
// surface – giving an ‘as-the-crow-flies’ distance between the points
template<class ResultType>
static inline ResultType haversine(ResultType lat1, ResultType lon1,
                                   ResultType lat2, ResultType lon2) {

    // XX probably not useful for comparison since it's a constant
    double R = 6371; // earth’s radius (mean radius = 6,371km)

    // a = sin^2(dlat / 2) + cos(lat1) * cos(lat2) * sin^2(dlon / 2)
    double sin_half_dLat = sin(to_radians<ResultType>(lat2 - lat1) / 2);
    double sin_half_dLon = sin(to_radians<ResultType>(lon2 - lon1) / 2);

    double cos_lat1 = cos(to_radians<ResultType>(lat1));
    double cos_lat2 = cos(to_radians<ResultType>(lat2));

    double a = sin_half_dLat * sin_half_dLat
        + cos_lat1 * cos_lat2 * sin_half_dLon * sin_half_dLon;

    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return R * c;
}

// Haversine distance for gps coordinates
template<class T>
struct HaversineDist {
    typedef False is_kdtree_distance;

    typedef True is_vector_space_distance;

    typedef T ElementType;
    typedef typename Accumulator<T>::Type ResultType;

    /**
     *  Compute the Haversine distance between two vectors.
     */
    template <typename Iterator1, typename Iterator2>
    ResultType operator()(Iterator1 a, Iterator2 b, size_t size, ResultType /* worst_dist */ = -1) const
    {
        // only defined for lat, lon vector can be max of 2
        assert(size == 2);

        return haversine<ResultType>(a[0], a[1], b[0], b[1]);
    }

};

#endif // !HAVERSINE_DIST_H
