/* Copyright 2014 Matthieu Tourne */

#ifndef FEATURES2D_H
#define FEATURES2D_H

#include <memory>
#include <vector>
#include <opencv2/features2d/features2d.hpp>

#include "photogram.h"

typedef std::vector<KeyPoint>   Keypoints;
typedef std::vector<DMatch>     Matches;

struct ImageFeatures {
    Keypoints           keypoints;

#ifdef USE_SIFT_GPU
    std::vector<float>  descriptors;
#else
    Mat                 descriptors;
#endif

    // serialization
    void write(FileStorage &fs) const;

    // deserialization
    void read(const FileNode &node);
};

typedef std::shared_ptr<ImageFeatures>  ImageFeaturesPtr;

// serialization
inline void write(FileStorage& fs, const std::string&,
                         const ImageFeatures& x) {
    x.write(fs);
}

// deserialization
inline void read(const FileNode& node, ImageFeatures& x,
                 const ImageFeatures& default_value = ImageFeatures()){
    if (node.empty())
        x = default_value;
    else
        x.read(node);
}

int get_features(const Mat img_gray, ImageFeatures& features);
int match_features(ImageFeatures &features1,
                   ImageFeatures &features2, Matches& match);
void matches2points(const Matches& matches,
                    ImageFeatures& features1, ImageFeatures& features2,
                    vector<Point2f>& pts1, vector<Point2f>& pts2);

bool get_putative_matches(const Matches &matches, const vector<char> &keypointsInliers,
                          Matches &output);

void write_matches_image(const Mat img1, const ImageFeatures &features1,
                         const Mat img2, const ImageFeatures &features2,
                         const Matches &matches,
                         const vector<char> &keypointMask = vector<char>(),
                         const string output = "matches.jpg");

#endif // !FEATURES2D_H
