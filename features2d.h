/* Copyright 2014 Matthieu Tourne */

#ifndef FEATURES2D_H
#define FEATURES2D_H

#include <memory>
#include <vector>
#include <opencv2/features2d/features2d.hpp>

#include "photogram.h"

typedef std::vector<KeyPoint>   Keypoints;
typedef std::vector<DMatch>     Matches;

typedef struct {
    Keypoints           keypoints;

#ifdef USE_SIFT_GPU
    std::vector<float>  descriptors;
#else
    Mat                 descriptors;
#endif

} ImageFeatures;

typedef std::shared_ptr<ImageFeatures>  ImageFeaturesPtr;

int get_features(const Mat img_gray, ImageFeatures& features);
int match_features(ImageFeatures &features1,
                   ImageFeatures &features2, Matches& match);
int get_F_matrix(ImageFeatures &features1, ImageFeatures &features2,
                 Matches &matches, Mat &F, vector<char> &keypointMask);
void matches2points(const Matches& matches,
                    ImageFeatures& features1, ImageFeatures& features2,
                    vector<Point2f>& pts1, vector<Point2f>& pts2);
void write_matches_image(const Mat img1, const ImageFeatures &features1,
                         const Mat img2, const ImageFeatures &features2,
                         const Matches &matches,
                         const vector<char> &keypointMask = vector<char>(),
                         const string output = "matches.jpg");

#endif // !FEATURES2D_H
