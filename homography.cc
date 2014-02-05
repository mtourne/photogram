/* Copyright 2014 Matthieu Tourne */

#include "photogram.h"

_INITIALIZE_EASYLOGGINGPP

#include <opencv2/calib3d/calib3d.hpp>
#include "features2d.h"

void help(void) {
    cout << "Usage: homography img1 img2" << endl
         << "\tReproject img2 onto img1";
}

int main(int argc, char** argv) {

    if (argc != 3) {
        help();
        return 1;
    }

    vector<ImageFeatures*> feature_list;
    vector<Mat*> images;

    for (int i = 1; i < argc; i++) {
        Mat *img = new Mat();
        Mat *img_gray = new Mat();

        *img = imread(argv[i]);
        cvtColor(*img, *img_gray, COLOR_BGR2GRAY);

        ImageFeatures *features = new ImageFeatures();
        features->img_gray = img_gray;
        features->img = img;
        features->filename = argv[i];

        // compute SIFT descriptors
        get_features(*img_gray, *features);

        feature_list.push_back(features);
     }

    ImageFeatures *features1 = feature_list[0];
    ImageFeatures *features2 = feature_list[1];

    Matches matches;
    vector<Point2f> pts1, pts2;
    Mat H;

    // match image descriptors
    match_features(*features1, *features2, matches);

    matches2points(matches, *features1, *features2, pts1, pts2);

    H = findHomography(pts1, pts2, CV_RANSAC);

    LOG(DEBUG) << "Homography matrix H: " << endl << H;

    Mat warped;
    // warp the perspective to fit onto img1
    warpPerspective(*features1->img, warped, H, features2->img->size());

    imwrite("img1_img2_warped.jpg", warped);

    return 0;
}
