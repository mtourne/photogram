/* Copyright 2014 Matthieu Tourne */

#ifndef FEATURES2D_H
#define FEATURES2D_H

#include "photogram.h"

#include <vector>
#include <opencv2/features2d/features2d.hpp>

typedef std::vector<KeyPoint>   Keypoints;
typedef std::vector<DMatch>     Matches;
typedef std::shared_ptr<Mat>    MatPtr;

typedef struct {
    MatPtr              img;
    MatPtr              img_gray;

    string              filename;

    Keypoints           keypoints;

#ifdef USE_SIFT_GPU
    std::vector<float>  descriptors;
    const string        method = "SiftGPU";
#else
    Mat                 descriptors;
    const string        method = "SiftOpenCV";
#endif
} ImageFeatures;


typedef std::shared_ptr<ImageFeatures>                  ImageFeaturesPtr;

int get_features(Mat& img_gray, ImageFeatures& features);
int match_features(ImageFeatures &features1,
                   ImageFeatures &features2, Matches& match);
int get_F_matrix(ImageFeatures &features1, ImageFeatures &features2,
                 Matches &matches, Mat &F, vector<char> &keypointMask);
void matches2points(const Matches& matches,
                    ImageFeatures& features1, ImageFeatures& features2,
                    vector<Point2f>& pts1, vector<Point2f>& pts2);
void write_matches_image(const ImageFeatures &features1, const ImageFeatures &features2,
                         const Matches &matches,
                         const string file_tag = "",
                         const vector<char> &keypointMask = vector<char>());

#endif // !FEATURES2D_H
