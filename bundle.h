/* Copyright 2014 Matthieu Tourne */

#ifndef BUNDLE_H
#define BUNDLE_H

#include <vector>

#include "image.h"
#include "image_pairs.h"

class Bundle {
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

    inline vector<ImagePair> get_image_pairs() const {
        return image_pairs;
    }

    inline ImagePair get_image_pair(const int i) const {
        return image_pairs[i];
    }

    void add_image(Image::ptr image);

    inline Image::ptr get_image(const int i) const {
        return images[i];
    }

    inline vector<Image::ptr> get_images() const {
        return images;
    }

    inline size_t image_count() const {
        return images.size();
    }

    // serialization
    void write(FileStorage& fs) const;

    // deserialization
    void read(const FileNode& node);


 private:
    // potentially this could be a hashmap indexed by camera
    // type, if there are multiple photos from different cams

    // XX (mtourne): I think it's possible to match different "K" from
    // different images (need to check).
    vector<ImagePair>   image_pairs;

    vector<Image::ptr>    images;
};

// serialization
inline void write(FileStorage& fs, const std::string&, const Bundle& x) {
    x.write(fs);
}

// deserialization
inline void read(const FileNode& node, Bundle& x,
                 const Bundle& default_value = Bundle()){
    if (node.empty())
        x = default_value;
    else
        x.read(node);
}

#endif // !BUNDLE_H
