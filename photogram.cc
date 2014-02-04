#include <iostream>
#include <stdio.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
// non-free sift detector
#include <opencv2/nonfree/features2d.hpp>

#include <Eigen/Dense>
#include <math.h>

#include "easyexif/exif.h"


using namespace std;
using namespace cv;

#define DEBUG 1

typedef Eigen::Matrix<double, 3, 3> IntrinsicMat;
typedef vector<KeyPoint> Keypoints;

void help(void) {
    cout << "Usage: photogram [image list]" << endl;
}

// debug exif data
void print_exif_data(EXIFInfo &data) {
    printf("Camera make       : %s\n", data.Make.c_str());
    printf("Camera model      : %s\n", data.Model.c_str());
    printf("Original date/time: %s\n", data.DateTimeOriginal.c_str());
    printf("Lens focal length : %f mm\n", data.FocalLength);
    printf("Image width       : %d\n", data.ImageWidth);
    printf("Image height      : %d\n", data.ImageHeight);
    printf("GPS Latitude      : %f deg (%f deg, %f min, %f sec %c)\n",
           data.GeoLocation.Latitude,
           data.GeoLocation.LatComponents.degrees,
           data.GeoLocation.LatComponents.minutes,
           data.GeoLocation.LatComponents.seconds,
           data.GeoLocation.LatComponents.direction);
    printf("GPS Longitude     : %f deg (%f deg, %f min, %f sec %c)\n",
           data.GeoLocation.Longitude,
           data.GeoLocation.LonComponents.degrees,
           data.GeoLocation.LonComponents.minutes,
           data.GeoLocation.LonComponents.seconds,
           data.GeoLocation.LonComponents.direction);
    printf("GPS Altitude      : %f m\n", data.GeoLocation.Altitude);
}

// read exif data for a file
int read_exif(const char *filename, EXIFInfo &exif_data) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        cerr << "Can't read file " << filename << endl;
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char buf[fsize];
    if (fread(buf, 1, fsize, fp) != fsize) {
        cerr << "Can't read file " << filename << endl;
        return -2;
    }
    fclose(fp);

    int code = exif_data.parseFrom(buf, fsize);

    if (code) {
        cerr << "Error parsing EXIF from file " <<
            filename << ", code: " << code << endl;
        return -3;
    }

    return 0;
}

// TODO (mtourne): grab from a list of cameras
int get_camera_sensor_size(const string camera_model,
                           float &width, float &height) {
    width = 6.17;
    height = 4.55;

    return 0;
}

// get the intrinsic matrix of the camera
int get_k_matrix_from_exif(const char *filename, Mat& img, IntrinsicMat &K) {
    EXIFInfo exif_data;
    int rc;

    rc = read_exif(filename, exif_data);
    if (rc != 0) {
        return rc;
    }

#ifdef DEBUG
    print_exif_data(exif_data);
#endif

    assert(exif_data.ImageWidth == img.cols &&
           exif_data.ImageHeight == img.rows);

    // ccd size in mm
    float width, height;

    rc = get_camera_sensor_size(exif_data.Model, width, height);
    if (rc != 0) {
        return rc;
    }

    // half fov in both directions
    // tan(tetha/2) = width / 2f
    double tan_half_x_fov = width / (2 * exif_data.FocalLength);
    double tan_half_y_fov = height / (2 * exif_data.FocalLength);

    // focal length in pixels :
    // f = (W/2) * (1 / tan(tetha/2))
    double f_x = img.cols / (2 * tan_half_x_fov);
    double f_y = img.rows / (2 * tan_half_y_fov);

    K = IntrinsicMat::Identity();
    K(0,0) = f_x;
    K(1,1) = f_y;

    // center of the sensor is set at width and height / 2
    K(0,2) = img.cols / 2;
    K(1,2) = img.rows / 2;

#ifdef DEBUG
    cout << "K intrinsic matrix: " << endl << K << endl;
#endif

    return 0;
}

int get_features(Mat& img_gray, Keypoints &keypoints, Mat &descriptors) {
    // TODO (mtourne): only need to instantiate detector / descriptor once

    // SIFT feature detector and feature extractor
    SiftFeatureDetector detector;
    SiftDescriptorExtractor extractor;

    // for surf, use this
    // SurfFeatureDetector detector;
    // SurfDescriptorExtractor extractor;

    detector.detect(img_gray, keypoints);
    extractor.compute(img_gray, keypoints, descriptors);

#if DEBUG
    cout << "keypoints: " << keypoints.size() << endl;
#endif

    return 0;

}

int main(int argc, char **argv) {

    if (argc < 2) {
        help();
        return 1;
    }

    Mat img;
    Mat img_gray;
    IntrinsicMat K;

    for (int i = 1; i < argc; i++) {
        img = imread(argv[i]);
        cvtColor(img, img_gray, COLOR_BGR2GRAY);

        // get instrinsic camera matrix K
        get_k_matrix_from_exif(argv[i], img, K);

        // compute SIFT features
        Mat descriptors;
        Keypoints keypoints;
        get_features(img_gray, keypoints, descriptors);

    }

    return 0;
}
