#ifndef IMAGE_PAIRS_H
#define IMAGE_PAIRS_H

#include "photogram.h"
#include "features2d.h"
#include "image.h"

#define MIN_FEATURE_MATCHES 50
#define MIN_INLIERS 50

// Image pair is a pair of images from the same camera at different point in the space
class ImagePair {
 public:
    ImagePair(Image::ptr image1, Image::ptr image2)
        : image1(image1), image2(image2), error("")
    {};
    ~ImagePair() {};

    bool compute_matches();

    // compute fundamental matrix
    bool compute_F_mat();

    // compute camera matrix P
    int compute_camera_mat();

    void print_matches() const;

    // TODO (mtourne): save / load of imagepair

 private:

    Image::ptr          image1;
    Image::ptr          image2;

    Matches             matches;

    // Fundamental Matrix between 2 images
    Mat                 F;

    // vector of inliers associated to matches
    // keypointsInlierns[i] != 0 => matches[i] good match
    vector<char>        keypointsInliers;
    unsigned int        inliers_count;

    string error;
};

#endif // !IMAGE
