/* Copyright 2014 Matthieu Tourne */

#ifndef IMAGE_H
#define IMAGE_H

#include "photogram.h"
#include "features2d.h"
#include "util.h"

class Image {
public:
    typedef std::shared_ptr<Image>  ptr;

    Image()
        : filename("NA"), name("NA")
    {};

    Image(const string filename) {
        set_filename(filename);
    }

    ~Image() {};

    inline void set_filename(const string file) {
        filename = file;
        name = remove_extension(basename(file));
    }

    inline void set_name(const string new_name) {
        name = new_name;
    }

    inline string get_name() const {
        return name;
    }

    inline void set_gps_coordinates(double lat, double lon) {
        coords.at<double>(0,0) = lat;
        coords.at<double>(0,1) = lon;
    }

    inline Mat get_coordinates() {
        return coords;
    }

    inline void set_camera_matrix(Mat K) {
        K = K;
    }

    inline Mat get_camera_matrix() const {
        return K;
    }

    Mat get_image();
    void add_transparency_layer(string filename);

    Mat get_image_gray();

    // TODO (mtourne) : implement
    bool parse_exif_data();

    ImageFeaturesPtr get_image_features();

    // serialization
    void write(FileStorage& fs) const;

    // deserialization
    void read(const FileNode& node);

    // keep all known images here, useful for serialization.
    static std::map<size_t, Image::ptr> all_images;
    static std::map<Image::ptr, size_t> all_images_index;
    static size_t image_count;

protected:
    Mat img;
    Mat img_gray;

    // file name
    string filename;

    // name
    string name;

    // invariant features
    ImageFeaturesPtr   features;

    // intrinsic camera matrix
    Mat K;

    // coordinates in 3d space, can be gps coords
    // used to create a pairlist
    Mat coords;
};

// serialization
inline void write(FileStorage& fs, const std::string&, const Image& x) {
    x.write(fs);
}

// deserialization
inline void read(const FileNode& node, Image& x,
                 const Image& default_value = Image()){
    if (node.empty())
        x = default_value;
    else
        x.read(node);
}

Mat dewarp_channels(const Mat input, const Mat Homography, Size output_size);

#endif
