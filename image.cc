#include "image.h"

MatPtr Image::get_image() {
    if (img) {
        // image already loaded
        return img;
    }

    // load on the fly image from file
    img.reset(new Mat());

    LOG(DEBUG) << "Loading image from file: " << filename;
    *img = imread(filename);
    if (img == 0) {
        LOG(ERROR) << "Unable to load image file: " << filename;
        img.reset();
        return MatPtr();
    }

    return img;
}

MatPtr Image::get_image_gray() {
    if (img_gray) {
        // gray image already loaded
        return img_gray;
    }

    // convert on the fly image gray from image
    MatPtr img;

    img = get_image();
    if (!img) {
        return MatPtr();
    }

    img_gray.reset(new Mat());
    LOG(DEBUG) << "Converting color to gray image";
    cvtColor(*img, *img_gray, COLOR_BGR2GRAY);

    return img_gray;
}

ImageFeaturesPtr Image::get_image_features() {
    if (features) {
        return features;
    }

    MatPtr image_gray = get_image_gray();

    if (!image_gray) {
        LOG(ERROR) << "Unable to retrieve gray image";
        return ImageFeaturesPtr();
    }

    features.reset(new ImageFeatures);

    // get SIFT like features
    int rc;
    LOG(DEBUG) << "Getting SIFT-like features";
    rc = get_features(*img_gray, *features);
    if (rc != 0) {
        LOG(ERROR) << "Unable to compute image features";
        features.reset();
        return ImageFeaturesPtr();
    }

    return features;
}
