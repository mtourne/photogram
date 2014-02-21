/* Copyright 2014 Matthieu Tourne */

#include "bundle.h"

void Bundle::add_image(Image::ptr image) {
    images.push_back(image);

    // TODO (mtourne): if coordinates are available
    // copy them in a coordinates matrix

    // this is for the L1/haversine flann matching
}

void Bundle::write(FileStorage& fs) const {
    LOG(DEBUG) << "Serializing Bundle";

    // avoid writing all the image data when pairs are in a bundle
    // just store the index

    std::map<Image::ptr, size_t> image_index_map;
    std::vector<std::pair<size_t, size_t>> image_pairs_index;

    // create a list of indexes
    for (size_t i = 0; i < images.size(); i++) {
        image_index_map[images[i]] = i;
    }

    fs << "{"
       << "images" << "[";

    for (auto image_ptr : images) {
        fs << *image_ptr;
    }
    fs << "]";

    fs << "image_pairs" << "[";
    for (auto image_pair : image_pairs) {
        Image::ptr img1 = image_pair.first();
        Image::ptr img2 = image_pair.second();

        size_t index1 = image_index_map[img1];
        size_t index2 = image_index_map[img2];

        LOG(DEBUG) << "Pair indexes, first: " << index1
                   << ", second: " << index2;

        fs << "{"
           << "idx1" << (int) index1
           << "idx2" << (int) index2
           << "pair_data";
        image_pair.write(fs, false);

        fs << "}";
    }

    fs << "]"
       << "}";
}

void Bundle::read(const FileNode& node) {

    LOG(DEBUG) << "Deserializing Bundle";

    FileNode n = node["images"];
    if (n.type() != FileNode::SEQ) {
        LOG(ERROR) << "images is not a sequence!!";
    }

    FileNodeIterator it = n.begin(), it_end = n.end();

    for( ; it != it_end; ) {
        Image::ptr img (new Image());

        // seems like this does an it++
        it >> *img;
        images.push_back(img);
    }

    n = node["image_pairs"];
    it = n.begin();
    it_end = n.end();

    for( ; it != it_end; ++it) {
        LOG(DEBUG) << "Deserializing bundled image pair";

        int idx1, idx2;
        (*it)["idx1"] >> idx1;
        (*it)["idx2"] >> idx2;

        Image::ptr img1 = images[idx1];
        Image::ptr img2 = images[idx2];

        ImagePair new_pair(img1, img2);

        new_pair.read((*it)["pair_data"], false);

        image_pairs.push_back(new_pair);
    }
}
