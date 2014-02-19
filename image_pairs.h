#ifndef IMAGE_PAIRS_H
#define IMAGE_PAIRS_H

#include "photogram.h"
#include "features2d.h"
#include "image.h"
#include "indexed_matches.h"

#define MIN_FEATURE_MATCHES 50
#define MIN_INLIERS 50

// Image pair is a pair of images from the same camera at different point in the space
class ImagePair {
 public:
    ImagePair() {};
    ImagePair(Image::ptr image1, Image::ptr image2)
        : image1(image1), image2(image2) {};

    ~ImagePair() {};

    bool compute_matches();

    // compute fundamental matrix
    bool compute_F_mat();

    // compute camera matrix P
    int compute_camera_mat();

    void print_matches() const;

    bool get_indexed_matches(vector<IndexedMatch> &indMatches) const;

    // serialization
    void write(FileStorage& fs) const;

    // deserialization
    void read(const FileNode& node);

    // TODO (mtourne): std::pair<Image::ptr, Image::ptr> might make sense
    inline Image::ptr first() const {
        return image1;
    }

    inline Image::ptr second() const {
        return image2;
    }

    // filter putative matches from a model (F matrix, homography ..)
    bool filterPutativeMatches();

    inline Matches get_matches() const {
        return matches;
    }

    inline void set_matches(Matches new_matches) {
        matches = new_matches;
    }

 private:

    Image::ptr          image1;
    Image::ptr          image2;

    Matches             matches;

    // Fundamental Matrix between 2 images
    Mat                 F;

    // vector of inliers associated to matches
    // keypointsInlierns[i] != 0 => matches[i] good match
    vector<char>        keypointsInliers;
    unsigned int        inliers_count;

    string error;
};

// serialization
inline void write(FileStorage& fs, const std::string&, const ImagePair& x) {
    x.write(fs);
}

// deserialization
inline void read(const FileNode& node, ImagePair& x,
                 const ImagePair& default_value = ImagePair()){
    if (node.empty())
        x = default_value;
    else
        x.read(node);
}

#endif // !IMAGE_PAIRS_H
