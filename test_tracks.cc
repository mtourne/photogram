#include "photogram.h"
#include "image_pairs.h"
#include "image.h"
#include "tracks.hpp"

_INITIALIZE_EASYLOGGINGPP

int main() {
    vector<ImagePair> vect;

    Image::ptr img1(new Image());
    img1->set_name("img1");
    Image::ptr img2(new Image());
    img2->set_name("img2");
    Image::ptr img3(new Image());
    img3->set_name("img3");

    ImagePair pair1(img1, img2);
    ImagePair pair2(img2, img3);

    /*
      img1 img2 img3
      0 -> 0 -> 0
      1 -> 1 -> 6
      2 -> 3
    */

    DMatch match;

    // pair1
    Matches matches1;

    match.trainIdx = 0;
    match.queryIdx = 0;
    matches1.push_back(match);

    match.trainIdx = 1;
    match.queryIdx = 1;
    matches1.push_back(match);

    match.trainIdx = 2;
    match.queryIdx = 3;
    matches1.push_back(match);

    pair1.set_matches(matches1);

    // pair2
    Matches matches2;

    match.trainIdx = 0;
    match.queryIdx = 0;
    matches2.push_back(match);

    match.trainIdx = 1;
    match.queryIdx = 6;
    matches2.push_back(match);

    pair2.set_matches(matches2);

    // list of img pairs
    vect.push_back(pair1);
    vect.push_back(pair2);

    TracksBuilder tracks_builder;
    tracks_builder.Build(vect);

    tracks_builder.ExportToStream(cout);

    //STLMAPTracks map_tracks;

    return 0;
}
