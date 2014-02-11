#include "image.h"

Mat Image::get_image() {
    if (img.data) {
        // image already loaded
        return img;
    }

    // load on the fly image from file
    LOG(DEBUG) << "Loading image from file: " << filename;
    img = imread(filename, CV_LOAD_IMAGE_UNCHANGED);
    if (!img.data) {
        throw std::runtime_error("Could not open file.");
    }

    LOG(DEBUG) << "Loaded image size: " << img.size()
               << ", channels: " << img.channels();

    return img;
}

void Image::add_transparency_layer(string filename) {
    Mat image = get_image();

    LOG(DEBUG) << "Loading alpha channel from file: " << filename;
    Mat mask = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (!mask.data) {
        throw std::runtime_error("Could not open file.");
    }

    vector<Mat> input(2);
    input[0] = image;
    input[1] = mask;
    merge(input, img);

    LOG(DEBUG) << "Image size: " << img.size()
               << ", channels: " << img.channels();
}

Mat Image::get_image_gray() {
    if (img_gray.data) {
        // gray image already loaded
        return img_gray;
    }

    // convert on the fly image gray from image
    Mat img_color;

    img_color = get_image();
    LOG(DEBUG) << "Converting color to gray image";
    cvtColor(img, img_gray, COLOR_BGR2GRAY);
    if (!img_gray.data) {
        throw std::runtime_error("Could not get gray image");
    }

    return img_gray;
}

ImageFeaturesPtr Image::get_image_features() {
    if (features) {
        return features;
    }

    Mat image_gray = get_image_gray();

    features.reset(new ImageFeatures);

    // get SIFT like features
    int rc;
    LOG(DEBUG) << "Getting SIFT-like features";
    rc = get_features(image_gray, *features);
    if (rc != 0) {
        LOG(ERROR) << "Unable to compute image features";
        features.reset();
        return ImageFeaturesPtr();
    }

    return features;
}

Mat dewarp_channels(const Mat input, const Mat H, Size output_size) {
    int channels = input.channels();
    vector<Mat> input_channels(channels);
    vector<Mat> output_channels(channels);

    split(input, input_channels);

    for (int i = 0; i < channels; i++) {
        warpPerspective(input_channels[i], output_channels[i], H, output_size);
    }

    Mat output;
    merge(output_channels, output);
    return output;
}
