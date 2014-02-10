/* Copyright 2014 Matthieu Tourne */

#ifndef BUNDLE_H
#define BUNDLE_H

#include <boost/utility.hpp>
#include <vector>

#include "image.h"
#include "image_pairs.h"

class Bundle : boost::noncopyable {
 public:
    Bundle() {};
    // create with an existing bundle
    Bundle(std::vector<ImagePair> image_pairs)
        : image_pairs(image_pairs)
    {};

    ~Bundle() {};

    inline void add_pair(ImagePair &pair) {
        image_pairs.push_back(pair);
    }

    inline size_t pair_count() const {
        return image_pairs.size();
    }

    void add_image(Image::ptr image);

    inline vector<Image::ptr> get_images() const {
        return images;
    }

    inline size_t image_count() const {
        return images.size();
    }

    // TODO (mtourne):
    // save / load structure

 private:
    // potentially this could be a hashmap indexed by camera
    // type, if there are multiple photos from different cams

    // XX (mtourne): I think it's possible to match different "K" from
    // different images (need to check).
    vector<ImagePair>   image_pairs;

    vector<Image::ptr>    images;
};

#endif // !BUNDLE_H
