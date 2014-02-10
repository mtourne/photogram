/* Copyright 2014 Matthieu Tourne */

#include "photogram.h"

_INITIALIZE_EASYLOGGINGPP

#include <opencv2/calib3d/calib3d.hpp>
#include "features2d.h"

#define VISUAL_DEBUG 0

void help(void) {
    cout << "Usage: homography img1 img2" << endl
         << "\tReproject img2 onto img1";
}

int main(int argc, char** argv) {

    if (argc != 3) {
        help();
        return 1;
    }

    vector<ImageFeaturesPtr> feature_list;

    for (int i = 1; i < argc; i++) {
        MatPtr img(new Mat());
        MatPtr img_gray(new Mat());

        *img = imread(argv[i]);
        if (!img) {
            LOG(FATAL) << "Unable to load image: " << argv[i];
            return 1;
        }

        cvtColor(*img, *img_gray, COLOR_BGR2GRAY);

        ImageFeaturesPtr features(new ImageFeatures());
        features->img_gray = img_gray;
        features->img = img;
        features->filename = argv[i];

        feature_list.push_back(features);
     }

#if 0
    // resize image1 to have the same scale as image2
    resize(*feature_list[0]->img, *feature_list[0]->img, feature_list[1]->img->size());
    resize(*feature_list[0]->img_gray, *feature_list[0]->img_gray, feature_list[1]->img->size());

    imwrite("rescaled_original.jpg", *feature_list[0]->img_gray);
#endif

    for (size_t i = 0; i < feature_list.size(); i++) {
        // compute SIFT descriptors
        get_features(*feature_list[i]->img_gray, *feature_list[i]);
    }


    ImageFeaturesPtr features1 = feature_list[0];
    ImageFeaturesPtr features2 = feature_list[1];

    Matches matches;
    vector<Point2f> pts1, pts2;
    Mat H;

    // match image descriptors
    match_features(*features1, *features2, matches);

    matches2points(matches, *features1, *features2, pts1, pts2);

    vector<unsigned char> status;
    vector<char> keypointMask;
    H = findHomography(pts1, pts2, CV_RANSAC, 3, status);

    LOG(DEBUG) << "Homography matrix H: " << endl << H;

    // print out RANSAC'ed matched keypoints
    keypointMask = vector<char> (status.begin(), status.end());

#if VISUAL_DEBUG
    write_matches_image(*features1, *features2, keypointMask);
#endif

#if 1
    // scale factor
    // x is the matrix cols
    // y is the matrix rows

     double scale_x = (double) features1->img->cols /
         (double) features2->img->cols;
     double scale_y = (double) features1->img->rows /
         (double) features2->img->rows;

     double scale = 8;

    // LOG(DEBUG) << "img1 rows: " << features1->img->rows
    //            << ", img2 rows: " << features2->img->rows << endl
    //            << "img1 cols: " << features1->img->cols
    //            << ", img2 cols: " << features2->img->cols << endl
    //            << "1 / scale factor x: " << scale_x
    //            << ", 1 / scale factor y: " << scale_y;

    Mat Scale = Mat::eye(3, 3, H.type());
    Scale.at<double>(0,0) = scale;
    // xx
    // Scale.at<double>(0,2) = 1 / scale;
    Scale.at<double>(1,1) = scale;
    // xx
    // Scale.at<double>(1,2) = 1 / scale;

    Mat P_up = Mat::eye(3, 3, H.type());
    P_up.at<double>(0,0) = 2;
    P_up.at<double>(0,2) = 0.5;
    P_up.at<double>(1,1) = 2;
    P_up.at<double>(1,2) = 0.5;

    LOG(DEBUG) << "Scale matrix : " << endl << Scale;

    H = H * Scale;

    LOG(DEBUG) << "Scaled Homography matrix H: " << endl << H;

    // LOG(DEBUG) << "Output image size: " << features1->img->size()
    //            << ", type: " << features1->img->type();

    // warp image1 to be registered onto image2 (keep aspect ratio)
    // XX (mtourne): only works with img_gray (opencv bug ?)
    Mat warped;
    Size size = features2->img->size();
    size.height *= scale;
    size.width *= scale;
    LOG(DEBUG) << "Output image size: " << size;

    warpPerspective(*features1->img_gray, warped, H, size);
    //warpPerspective(*features2->img_gray, warped, P_up, size);
#else

    // warp to register image1 onto image2
    Mat warped;
    warpPerspective(*features1->img_gray, warped, H, features2->img->size());
#endif

    imwrite("img1_img2_warped.tif", warped);

    return 0;
}
