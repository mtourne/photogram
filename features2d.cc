/* Copyright 2014 Matthieu Tourne */

#include <sstream>
#include <string>

#include <opencv2/features2d/features2d.hpp>
// non-free sift detector
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "photogram.h"
#include "features2d.h"
#include "util.h"

#ifdef USE_SIFT_GPU
#include "sift_gpu_wrapper.h"
#endif

SiftFeatureDetector opencv_sift_detector;
SiftDescriptorExtractor opencv_sift_extractor;
FlannBasedMatcher flann_matcher;



// for surf, use this
// SurfFeatureDetector opencv_surf_detector;
// SurfDescriptorExtractor opencv_surf_extractor;

int get_features(Mat img_gray, ImageFeatures &features) {

#ifdef USE_SIFT_GPU
    LOG(DEBUG) << "using SIFT gpu";

    SiftGPUWrapper* siftgpu = SiftGPUWrapper::getInstance();
    siftgpu->detect(img_gray, features.keypoints, features.descriptors);

    LOG(DEBUG) << "feature descriptors, count: " << features.descriptors.size();

#else
    LOG(DEBUG) << "using opencv SIFT";

    opencv_sift_detector.detect(img_gray, features.keypoints);
    opencv_sift_extractor.compute(img_gray, features.keypoints, features.descriptors);

    LOG(DEBUG) << "Found " << features.keypoints.size() << " features";
#endif

    return 0;
}

int match_features(ImageFeatures &features1,
                   ImageFeatures &features2, Matches &matches) {

#ifdef USE_SIFT_GPU
    LOG(DEBUG) << "Using GPU bruteforce matcher";

    SiftGPUWrapper* siftgpu = SiftGPUWrapper::getInstance();
    // each descriptor is 128 element with SIFT
    // num is the number of descriptors ..
    siftgpu->match(features1.descriptors, features1.descriptors.size() / 128,
        features2.descriptors, features2.descriptors.size() / 128,
        &matches);

#else
    LOG(DEBUG) << "Using a FLANN based matcher";

    flann_matcher.match(features1.descriptors, features2.descriptors, matches);
#endif

    LOG(DEBUG) << "Matches: " << matches.size();

    return 0;
}



// very simple way to get good matches
void get_good_matches(Matches &matches, Matches &good_matches) {
    double max_dist = 0; double min_dist = 1000.0;
    //-- Quick calculation of max and min distances between keypoints
    for(unsigned int i = 0; i < matches.size(); i++) {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    LOG(DEBUG) << "Matches, max dist: " << max_dist;
    LOG(DEBUG) << "Matches, min dist: " << min_dist;

    for(unsigned int i = 0; i < matches.size(); i++) {
        if (matches[i].distance <= max(4 * min_dist, 0.02)) {
            good_matches.push_back(matches[i]);
        }
    }

    LOG(DEBUG) << "Good Matches : " << good_matches.size();
}

void matches2points(const Matches& matches,
                    ImageFeatures& features1, ImageFeatures& features2,
                    vector<Point2f>& pts1, vector<Point2f>& pts2) {
    pts1.clear();
    pts2.clear();
    pts1.reserve(matches.size());
    pts2.reserve(matches.size());

    for (size_t i = 0; i < matches.size(); i++) {
        const DMatch& match = matches[i];
        pts1.push_back(features1.keypoints[match.queryIdx].pt);
        pts2.push_back(features2.keypoints[match.trainIdx].pt);
    }

    LOG(DEBUG) << "points1: " << pts1.size() << ", points2: " << pts2.size();
}

void write_matches_image(const Mat img1, const ImageFeatures &features1,
                         const Mat img2, const ImageFeatures &features2,
                         const Matches &matches,
                         const vector<char> &keypointMask,
                         const string output) {
    Mat img_matches;
    drawMatches(img1, features1.keypoints,
                img2, features2.keypoints,
                matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                keypointMask, DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    LOG(DEBUG) << "Writing image: " << output;
    imwrite(output.c_str(), img_matches);
}
