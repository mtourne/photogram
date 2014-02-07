#ifndef IMAGE_PAIRS_H
#define IMAGE_PAIRS_H

#include "photogram.h"
#include "features2d.h"

#define MIN_FEATURE_MATCHES 8
#define MIN_INLIERS 8

// Image pair is a pair of images from the same camera at different point in the space
class ImagePair {
 public:
    ImagePair(ImageFeaturesPtr features1, ImageFeaturesPtr features2, Mat33 K)
        : features1(features1), features2(features2), K(K), error("")
    {};
    ~ImagePair() {};

    int compute_matches();

    // compute fundamental matrix
    int compute_F_mat();

    void print_matches() const;

    // TODO (mtourne): save / load of imagepair

 private:

    ImageFeaturesPtr    features1;
    ImageFeaturesPtr    features2;

    Matches             matches;

    // an image pair need to have the same intrinsinc cam
    Mat33               K;
    Mat                 F;

    // vector of inliers associated to matches
    // keypointsInlierns[i] != 0 => matches[i] good match
    vector<char>        keypointsInliers;
    unsigned int        inliers_count;

    string error;
};

#endif
