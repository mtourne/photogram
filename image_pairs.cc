#include <opencv2/calib3d/calib3d.hpp>

#include "image_pairs.h"

int ImagePair::compute_matches() {
    match_features(*features1, *features2, matches);

    if (matches.size() < MIN_FEATURE_MATCHES) {
        LOG(DEBUG) << "Not enough matches: " << matches.size()
                   << ", at least " << MIN_FEATURE_MATCHES << "needed.";
        return 1;
    }

    return 0;
}

int ImagePair::compute_F_mat() {
    // use RANSAC to get F matrix
    // return better_matches for all the matches that make sense

    vector<Point2f> pts1, pts2;
    vector<unsigned char> status;

    matches2points(matches, *features1, *features2, pts1, pts2);

    F = findFundamentalMat(pts1, pts2, FM_RANSAC, 1, 0.99, status);

    // XX (mtourne): stupid vector<unsigned char> to vector<char> conversion
    keypointsInliers = vector<char> (status.begin(), status.end());

    inliers_count = countNonZero(keypointsInliers);

    LOG(DEBUG) << "F matrix: " << endl << F;

    LOG(DEBUG) << "Fundamental mat is keeping " << inliers_count << " / "
               << keypointsInliers.size();

    if (inliers_count < MIN_INLIERS) {
        LOG(DEBUG) << "Not enough inliers: " << inliers_count
                   << ", at least " << MIN_INLIERS << "needed.";
        return 1;
    }

    return 0;
}

void ImagePair::print_matches() const {
    if (keypointsInliers.size() > 0) {
        write_matches_image(*features1, *features2, matches, "RANSAC" , keypointsInliers);
    } else {
        write_matches_image(*features1, *features2, matches, "ALL_MATCHES");
    }
}
