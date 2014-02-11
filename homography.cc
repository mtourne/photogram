/* Copyright 2014 Matthieu Tourne */

#include "photogram.h"

_INITIALIZE_EASYLOGGINGPP

#include <opencv2/calib3d/calib3d.hpp>

#include "tclap/CmdLine.h"

#include "features2d.h"
#include "image.h"

#define VISUAL_DEBUG 0

// TODO (mtourne): add a cli argument to let override the scale

// Try to approximate a good scaling factor from the number of pixels
// this works assuming the viewport is similar in the two images.
static double get_scale(Size truth_size, Size img_size) {
    // most of the time img is going to be bigger than truth
    unsigned int ratio_w;
    unsigned int ratio_h;
    unsigned int ratio_max;
    double scale;

    if (img_size.width >= truth_size.width) {
        ratio_w = img_size.width / truth_size.width;
        ratio_h = img_size.height / truth_size.height;
    } else {
        ratio_w = truth_size.width / img_size.width;
        ratio_h = truth_size.height / img_size.height;
    }

    ratio_max = max(ratio_w, ratio_h);

    scale = upper_power_of_two(ratio_max);

    if (img_size.width < truth_size.width) {
        scale = 1 / scale;
    }

    return scale;
}

int main(int argc, char** argv) {

    try {
        TCLAP::CmdLine cmd("Find a homagraphy between ground truth and random image", ' ', "0.1");

        TCLAP::ValueArg<std::string> ground_truth("", "truth", "Ground truth", true, "", "filename");
        cmd.add(ground_truth);
        TCLAP::ValueArg<std::string> image("", "image", "Image to map onto ground truth", true, "", "filename");
        cmd.add(image);
        TCLAP::ValueArg<std::string> output("", "output", "Prefix for output image", true, "", "name");
        cmd.add(output);

        cmd.parse(argc, argv);

        std::string ground_truth_filename = ground_truth.getValue();
        std::string img_filename = image.getValue();
        std::string output_filename = output.getValue() + ".tif";

        Image::ptr img(new Image(img_filename));
        Image::ptr truth(new Image(ground_truth_filename));

        ImageFeaturesPtr img_features = img->get_image_features();
        ImageFeaturesPtr truth_features = truth->get_image_features();

        Matches matches;
        vector<Point2f> truth_pts, img_pts;
        Mat H;

        // match image descriptors
        match_features(*img_features, *truth_features, matches);

        matches2points(matches, *img_features, *truth_features, img_pts, truth_pts);

        vector<unsigned char> status;
        H = findHomography(img_pts, truth_pts, CV_RANSAC, 3, status);

        LOG(DEBUG) << "Homography matrix H: " << endl << H;

#if VISUAL_DEBUG
        // print out RANSAC'ed matched keypoints
        vector<char> keypointMask;

        keypointMask = vector<char> (status.begin(), status.end());

        write_matches_image(img->get_image_gray(), *img_features,
                            truth->get_image_gray(), *truth_features,
                            matches,
                            keypointMask, "matches_RANSAC.jpg");
#endif


        Size truth_size = truth->get_image_gray()->size();
        Size img_size = img->get_image_gray()->size();

        // find closest power of 2 ratio between the two images
        double scale = get_scale(truth_size, img_size);

        LOG(DEBUG) << "Scaling output image by: " << scale;

        Mat Scale = Mat::eye(3, 3, H.type());
        Scale.at<double>(0,0) = scale;
        Scale.at<double>(1,1) = scale;

        LOG(DEBUG) << "Scale matrix: " << endl << Scale;

        H = Scale * H;

        LOG(DEBUG) << "Scaled Homography matrix H: " << endl << H;

        truth_size.height *= scale;
        truth_size.width *= scale;
        LOG(DEBUG) << "Output image size: " << truth_size;

        Mat warped;
        warpPerspective(*img->get_image_gray(), warped, H, truth_size);

        imwrite(output_filename, warped);

        return 0;

    } catch (TCLAP::ArgException &e)  {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}
