/* Copyright 2014 Matthieu Tourne */

#include <sstream>
#include <string>

#include <opencv2/features2d/features2d.hpp>
// non-free sift detector
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "photogram.h"
#include "features2d.h"

#ifdef USE_SIFT_GPU
#include "sift_gpu_wrapper.h"
#endif

SiftFeatureDetector opencv_sift_detector;
SiftDescriptorExtractor opencv_sift_extractor;
FlannBasedMatcher flann_matcher;



// for surf, use this
// SurfFeatureDetector opencv_surf_detector;
// SurfDescriptorExtractor opencv_surf_extractor;

int get_features(Mat& img_gray, ImageFeatures &features) {

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

std::string basename(const std::string& pathname) {
    return {std::find_if(pathname.rbegin(), pathname.rend(),
                         [](char c) { return c == '/'; }).base(),
            pathname.end()};
}

std::string remove_extension(const std::string& pathname) {
    string s = pathname;
    s.erase(s.find_last_of("."), string::npos);
    return s;
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

// use RANSAC to get F matrix
// return better_matches for all the matches that make sense
int get_F_matrix(ImageFeatures &features1, ImageFeatures &features2,
                 Matches &matches, Mat &F, vector<char> &keypointMask) {
    vector<Point2f> pts1, pts2;
    vector<unsigned char> status;

    for(unsigned int i = 0; i < matches.size(); i++) {
        pts1.push_back(features1.keypoints[matches[i].queryIdx].pt);
        pts2.push_back(features2.keypoints[matches[i].trainIdx].pt);
    }

    LOG(DEBUG) << "points1: " << pts1.size() << ", points2: " << pts2.size();

    F = findFundamentalMat(pts1, pts2, FM_RANSAC, 1, 0.99, status);

    // XX (mtourne): stupid vector<unsigned char> to vector<char> conversion
    keypointMask = vector<char> (status.begin(), status.end());

    LOG(DEBUG) << "F matrix: " << endl << F;

    LOG(DEBUG) << "Fundamental mat is keeping " << countNonZero(keypointMask) << " / "
               << keypointMask.size();

    return 0;
}

void write_matches_image(ImageFeatures &features1,
                         ImageFeatures &features2, Matches &matches,
                         const vector<char> &keypointMask = vector<char>()) {
    Mat img_matches;
    ostringstream ss;

    Matches good_matches;

    // get_good_matches(matches, good_matches);

    // TODO (mtourne): implement as set_filename() for Features2D class
    ss << "matches_" << remove_extension(basename(features1.filename)) << "_"
       << remove_extension(basename(features2.filename)) << "_"
       << features1.method << ".jpg";

    // sanity
    for( size_t m = 0; m < matches.size(); m++ ) {
        int i1 = matches[m].queryIdx;
        int i2 = matches[m].trainIdx;
        if (i1 >= features1.keypoints.size()) {

            cout << "i1: " << i1 << endl;
            cout << "matches / keypoints1 issue, size: " << features1.keypoints.size() << endl;
        }
        if (i2 >= features2.keypoints.size()) {
            cout << "id2: " << i2 << endl;
            cout << "matches / keypoints2 issue, size: " << features2.keypoints.size() << endl;
        }
    }

    drawMatches(*features1.img_gray, features1.keypoints,
                *features2.img_gray, features2.keypoints,
                matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                keypointMask, DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    LOG(DEBUG) << "Writing image: " << ss.str();

    imwrite(ss.str().c_str(), img_matches);
}
