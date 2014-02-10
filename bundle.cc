/* Copyright 2014 Matthieu Tourne */

#include "bundle.h"

void Bundle::add_image(Image::ptr image) {
    images.push_back(image);

    // TODO (mtourne): if coordinates are available
    // copy them in a coordinates matrix
}
