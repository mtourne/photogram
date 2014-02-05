/* Copyright 2014 Matthieu Tourne */

#ifndef FEATURES2D_H
#define FEATURES2D_H

#include <vector>
#include <opencv2/features2d/features2d.hpp>

typedef vector<KeyPoint> Keypoints;
typedef vector<DMatch>   Matches;

typedef struct {
    Mat             *img;
    Mat             *img_gray;

    string          filename;

    Keypoints       keypoints;

#ifdef USE_SIFT_GPU
    vector<float>   descriptors;
    const string    method = "SiftGPU";
#else
    Mat             descriptors;
    const string    method = "SiftOpenCV";
#endif
} ImageFeatures;

// todo (mtourne) : match is 2 images
typedef struct {
    Matches         matches;
    Keypoints       good_keypoints;

    vector<char>    keypointMask;
} ImageMatch;

int get_features(Mat& img_gray, ImageFeatures& features);
int match_features(ImageFeatures &features1,
                   ImageFeatures &features2, Matches& match);
int get_F_matrix(ImageFeatures &features1, ImageFeatures &features2,
                 Matches &matches, Mat &F, vector<char> &keypointMask);
void matches2points(const Matches& matches,
                    ImageFeatures& features1, ImageFeatures& features2,
                    vector<Point2f>& pts1, vector<Point2f>& pts2);
void write_matches_image(ImageFeatures &features1,
                         ImageFeatures &features2, Matches &matches,
                         const string file_tag = "",
                         const vector<char> &keypointMask = vector<char>());

#endif // !FEATURES2D_H
